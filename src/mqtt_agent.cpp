// Copyright (c) 2024 Alberto J. Tudela Roldán
// Copyright (c) 2024 Grupo Avispa, DTE, Universidad de Málaga
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>

#include "mqtt_dsr_agent/campero_types.hpp"
#include "mqtt_dsr_agent/json_messages.hpp"
#include "mqtt_dsr_agent/mqtt_agent.hpp"

MqttAgent::MqttAgent(
  const std::string agent_name, int agent_id,
  const std::string & server_address, const std::string & client_id)
: agent_name_(agent_name), agent_id_(agent_id), client_(server_address, client_id)
{
  /* ----------------------------------------  DSR  -------------------- -------------------- */
  // Register types for signals
  qRegisterMetaType<uint64_t>("uint64_t");
  qRegisterMetaType<std::string>("std::string");
  qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
  qRegisterMetaType<DSR::SignalInfo>("DSR::SignalInfo");

  // Create the DSR graph
  G_ = std::make_shared<DSR::DSRGraph>(agent_name, agent_id, "");

  // Add connection signals
  QObject::connect(
    G_.get(), &DSR::DSRGraph::update_node_signal, this, &MqttAgent::node_updated);
  QObject::connect(
    G_.get(), &DSR::DSRGraph::update_node_attr_signal, this, &MqttAgent::node_attributes_updated);
  QObject::connect(
    G_.get(), &DSR::DSRGraph::update_edge_signal, this, &MqttAgent::edge_updated);
  QObject::connect(
    G_.get(), &DSR::DSRGraph::update_edge_attr_signal, this, &MqttAgent::edge_attributes_updated);
  QObject::connect(
    G_.get(), &DSR::DSRGraph::del_node_signal, this, &MqttAgent::node_deleted);
  QObject::connect(
    G_.get(), &DSR::DSRGraph::del_edge_signal, this, &MqttAgent::edge_deleted);

  /* ----------------------------------------  MQTT  -------------------- -------------------- */
  conn_options_.set_clean_session(false);
  client_.set_callback(*this);
}

MqttAgent::~MqttAgent()
{
  G_.reset();
  disconnect();
}

void MqttAgent::set_credentials(const std::string & username, const std::string & password)
{
  conn_options_.set_user_name(username);
  conn_options_.set_password(password);
}

bool MqttAgent::connect()
{
  bool connected = false;
  try {
    mqtt::token_ptr conntok = client_.connect(conn_options_);
    conntok->wait();
    std::cout << "Connected to the MQTT broker." << std::endl;
    connected = true;
  } catch (const mqtt::exception & exc) {
    std::cerr << "Error connecting to the MQTT broker: " << exc.what() << std::endl;
  }
  return connected;
}

void MqttAgent::disconnect()
{
  try {
    mqtt::token_ptr conntok = client_.disconnect();
    conntok->wait();
    std::cout << "Disconnected from the MQTT broker." << std::endl;
  } catch (const mqtt::exception & exc) {
    std::cerr << "Error disconnecting from the MQTT broker: " << exc.what() << std::endl;
  }
}

void MqttAgent::set_topics(const std::vector<std::string> & topics)
{
  topics_ = topics;
}

/* ----------------------------------------  DSR  -------------------- -------------------- */
// Callbacks called when the DSR graph is changed
void MqttAgent::node_updated(std::uint64_t /*id*/, const std::string & /*type*/)
{
}

void MqttAgent::node_attributes_updated(
  uint64_t /*id*/, const std::vector<std::string> & /*att_names*/)
{
}

void MqttAgent::edge_updated(
  std::uint64_t from, std::uint64_t to, const std::string & type)
{
  if (type == "interacting"){
    auto robot_node = G_->get_node(from);
    auto prob_person_node = G_->get_node(to);
    if(robot_node.has_value() && robot_node.value().name() == "robot"
      && prob_person_node.has_value() && prob_person_node.value().type() == "person"){
        control = true;
        person_node = prob_person_node;
        std::cout << "Robot interacting with " << person_node.value().name() << std::endl;
      }
  }

  if (type == "in"){
    auto room_node = G_->get_node(to);
    auto prob_person_node = G_->get_node(from);
    if(room_node.has_value() && room_node.value().name() == "bed"
      && prob_person_node.has_value() && prob_person_node.value().type() == "person"){
        control = true;
        person_node = prob_person_node;
        std::string command = "mosquitto_pub -h 192.168.0.140 -p 1883 -t \"Sensor/Control\" -m \"OnSensor\"";
        system(command.c_str());
        std::cout << "Person" << person_node.value().name() << std::endl;
    }
    else if(room_node.has_value() && room_node.value().name() == "bathroom"
      && prob_person_node.has_value() && prob_person_node.value().type() == "person"){
        control = true;
        person_node = prob_person_node;
        std::cout << "Person" << person_node.value().name() << " in bathroom" << std::endl;
    }
  }
}

void MqttAgent::edge_attributes_updated(
  std::uint64_t /*from*/, std::uint64_t /*to*/,
  const std::string & /*type*/, const std::vector<std::string> & /*att_names*/)
{
}

void MqttAgent::node_deleted(std::uint64_t /*id*/)
{
}

void MqttAgent::edge_deleted(
  std::uint64_t /* from*/, std::uint64_t /*to*/, const std::string & /*edge_tag*/)
{
}

/* ----------------------------------------  MQTT -------------------- -------------------- */

void MqttAgent::reconnect(int delay)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(delay));
  try {
    client_.connect(conn_options_, nullptr, *this);
  } catch (const mqtt::exception & exc) {
    std::cerr << "Error: " << exc.what() << std::endl;
    exit(1);
  }
  if (edge_tag == "in"){
    std::cout << "Delete edge in between " << from << " and " << to << std::endl;
    person_node = {};
    std::string command = "mosquitto_pub -h 192.168.0.140 -p 1883 -t \"Sensor/Control\" -m \"OffSensor\"";
    system(command.c_str());
    control = false;
  }
}

void MqttAgent::on_success(const mqtt::token & tok)
{
  std::cout << "Connection Success" << std::endl;
  if (tok.get_message_id() != 0) {
    std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
  }
  std::cout << std::endl;
}

void MqttAgent::on_failure(const mqtt::token & tok)
{
  std::cout << "Connection attempt failed" << std::endl;
  if (tok.get_message_id() != 0) {
    std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
  }
  std::cout << std::endl;
  if (++nretry_ > N_RETRY_ATTEMPTS) {
    exit(1);
  }
  reconnect(2500);
}

void MqttAgent::connected(const std::string & cause)
{
  if (!cause.empty()) {
    std::cout << "cause: " << cause << std::endl;
  }
  std::cout << "Connection success" << std::endl;
  std::cout << "Subscribing to topics ";

  for (const auto & topic : topics_) {
    std::cout << "'" << topic << "', ";
    client_.subscribe(topic, QOS);
  }
  std::cout << std::endl;
}

void MqttAgent::connection_lost(const std::string & cause)
{
  std::cout << "Connection lost" << std::endl;
  if (!cause.empty()) {
    std::cout << "cause: " << cause << std::endl;
  }

  std::cout << "Reconnecting..." << std::endl;
  nretry_ = 0;
  reconnect(2500);
}

void MqttAgent::message_arrived(mqtt::const_message_ptr msg)
{
  /*
  if (msg->get_topic() == "cma/person/positional") {
    json j = json::parse(msg->get_payload_str());
    float distancia = j["distance"].get<float>();
    int time_stamp = j["timestamp"].get<int>();
    std::cout << "Detected person\n"
              << "At program time: " << time_stamp << "ms \n"
              << "At: " << distancia << "m" << std::endl;
    // Insertar las cosas de los nodos cuando hay una persona interacting
    if (mqtt_agent->person_node.has_value() && mqtt_agent->control) {
      std::cout << "Insert attribute to person: " << mqtt_agent->person_node.value().name() <<
        std::endl;
      mqtt_agent->insert_attribute<distancia_att, float>(
        mqtt_agent->person_node.value().name(), distancia);
      mqtt_agent->insert_attribute<distanciaTime_att, int>(
        mqtt_agent->person_node.value().name(), time_stamp);
    }
  }*/

  if (msg->get_topic() == "cma/person/vitals") {
    auto sensor = RespiratoryHeartbeatSensor(json::parse(msg->get_payload_str()));

/*
  if (msg->get_topic() == "cma/person/vitals") {
      // Insertar las cosas de los nodos cuando hay una persona interacting
      if (mqtt_agent->person_node.has_value() && mqtt_agent->control) {
        mqtt_agent->insert_attribute<heartRate_att, float>(
          mqtt_agent->person_node.value().name(), heart);
        mqtt_agent->insert_attribute<breathRate_att, float>(
          mqtt_agent->person_node.value().name(), breath);
        mqtt_agent->insert_attribute<vitalsTime_att, int>(
          mqtt_agent->person_node.value().name(), timestamp);
      }*/
    }
}

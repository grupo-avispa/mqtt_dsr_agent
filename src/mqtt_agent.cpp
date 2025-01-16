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
#include <chrono>

#include "mqtt_dsr_agent/campero_types.hpp"
#include "mqtt_dsr_agent/json_messages.hpp"
#include "mqtt_dsr_agent/mqtt_agent.hpp"

MqttAgent::MqttAgent(
  const int & agent_id, const std::string & agent_name,
  const std::string & source, const std::string & topic, const std::string & message_type,
  const std::string & parent_node, const std::string & sensor_name,
  const std::string & server_address,
  const std::string & client_id)
: agent_id_(agent_id), agent_name_(agent_name), source_(source), topic_(topic),
  message_type_(message_type), parent_node_name_(parent_node), sensor_name_(sensor_name),
  server_address_(server_address), client_id_(client_id), client_(server_address_, client_id_)
{
  /* ----------------------------------------  DSR  ---------------------------------------- */
  // Register types for signals
  qRegisterMetaType<uint64_t>("uint64_t");
  qRegisterMetaType<std::string>("std::string");
  qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
  qRegisterMetaType<DSR::SignalInfo>("DSR::SignalInfo");

  // Create the DSR graph
  G_ = std::make_shared<DSR::DSRGraph>(agent_name_, agent_id_, "");

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

  /* --------------------  MQTT  --------------------*/
  std::cout << std::endl << " Starting client configuration ...";
  conn_options_.set_clean_session(false);
  client_.set_callback(*this);
  std::cout << std::endl << " Finished client configuration ...";
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

// void MqttAgent::set_topics(const std::vector<std::string> & topics)
// {
//     topics_ = topics;
// }

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
  // if (type == "interacting"){
  //     auto robot_node = G_->get_node(from);
  //     auto prob_person_node = G_->get_node(to);
  //     if(robot_node.has_value() && robot_node.value().name() == "robot"
  //         && prob_person_node.has_value() && prob_person_node.value().type() == "person"){
  //         control_ = true;
  //         person_node = prob_person_node;
  //         std::cout << "Robot interacting with " << person_node.value().name() << std::endl;
  //         }
  // }

  if (type == "in" && message_type_ == "RespiratoryHeartbeatSensor") {
    parent_node_ = G_->get_node(parent_node_name_);
    if (!parent_node_.has_value()) {
      std::cout << "ERROR: Could not get Parent node [" << parent_node_name_ << "]"
                << std::endl;
      return;
    }
    auto room_node = G_->get_node(to);
    auto prob_person_node = G_->get_node(from);
    if (room_node.has_value() && room_node.value().name() == parent_node_.value().name() &&
      prob_person_node.has_value() && prob_person_node.value().type() == "person")
    {
      control_ = true;
      person_node_ = prob_person_node;
      // send control msg to sensor
      const char * payload = "OnSensor";
      std::string control_topic = topic_ + "/control";
      std::cout << "Control Topic: " << control_topic << std::endl;
      client_.publish(control_topic, payload, strlen(payload), QOS, false);
      std::cout << "Person: " << person_node_.value().name() << std::endl;
      std::cout << "FMCW Sensor measurement has started...:" << std::endl;
    }
    // else if(room_node.has_value() && room_node.value().name() == "bathroom"
    //     && prob_person_node.has_value() && prob_person_node.value().type() == "person"){
    //     control = true;
    //     person_node = prob_person_node;
    //     std::cout << "Person" << person_node.value().name() << " in bathroom" << std::endl;
    // }
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

void MqttAgent::edge_deleted(std::uint64_t from, std::uint64_t to, const std::string & edge_tag)
{
  // if (edge_tag == "interacting"){
  //     std::cout << "Delete edge interacting between " << from << " and " << to << std::endl;
  //     person_node = {};
  //     control = false;
  // }
  if (edge_tag == "in") {
    std::cout << "Delete edge in between " << from << " and " << to << std::endl;
    auto from_node = G_->get_node(from);
    auto to_node = G_->get_node(to);
    if (from_node.has_value() && person_node_.has_value() && to_node.has_value() &&
      parent_node_.has_value() && (to_node.value().name() == parent_node_.value().name()) &&
      (from_node.value().name() == person_node_.value().name()))
    {
      std::cout << "Delete person in bed" << std::endl;
      person_node_ = {};
      const char * payload = "OffSensor";
      std::string control_topic = topic_ + "/control";
      client_.publish("Sensor/Control", payload, strlen(payload), QOS, false);
      control_ = false;
      std::cout << "Person has left the room, stop measuring ..." << std::endl;
    }
  }
}

template<typename T>
void MqttAgent::sensor_data_to_dsr(const T & data)
{
  // First check data is valid
  if (data.heartrate <= 30 || data.breathrate <= 10) {
    return;
  }
  // Check if sensor node has been created
  auto sensor_node = G_->get_node(sensor_name_);
  // Get timestamp (better to do on local machine due to clock mis-synchronizations)
  auto now = std::chrono::system_clock::now();
  int timestamp = static_cast<int>(std::chrono::duration_cast<
      std::chrono::seconds>(now.time_since_epoch()).count());
  if (!sensor_node.has_value()) {
    sensor_node.emplace(DSR::Node::create<sensor_node_type>(sensor_name_));
    // Check type of msg
    if (message_type_ == "RespiratoryHeartbeatSensor") {
      // Parse vital parameters, update the sensor node and insert it
      G_->add_or_modify_attrib_local<heartrate_att>(sensor_node.value(), data.heartrate);
      G_->add_or_modify_attrib_local<breathrate_att>(sensor_node.value(), data.breathrate);
      G_->add_or_modify_attrib_local<timestamp_att>(sensor_node.value(), timestamp);
      if (auto id = G_->insert_node(sensor_node.value()); id.has_value()) {
        std::cout << "Inserted sensor node [" << sensor_name_ << "] in the graph."
                  << std::endl;
        if (!parent_node_.has_value()) {
          std::cout << "ERROR: Could not get Parent node [" << parent_node_name_ << "]"
                    << std::endl;
          return;
        }
        // Set "IN" edge between room and sensor
        auto edge =
          DSR::Edge::create<in_edge_type>(sensor_node.value().id(), parent_node_.value().id());
        if (G_->insert_or_assign_edge(edge)) {
          std::cout << "Inserted edge between [" << sensor_name_ << "] and ["
                    << parent_node_name_ << "]" << std::endl;
        }
        // Set "MEASURING" edge between sensor and person
        auto edges_in = G_->get_node_edges_by_type(parent_node_.value(), "in");
        if (person_node_.has_value() && sensor_node.has_value()) {
          auto edge_measure = DSR::Edge::create<measuring_edge_type>(
            sensor_node.value().id(), person_node_.value().id());
          if (G_->insert_or_assign_edge(edge_measure)) {
            std::cout << "Inserted edge between [" << sensor_name_ << "] and ["
                      << person_node_.value().name() << "]" << std::endl;
          }
        }
      }
    }
  } else {
    // Check type of msg
    if (message_type_ == "RespiratoryHeartbeatSensor") {
      G_->add_or_modify_attrib_local<heartrate_att>(sensor_node.value(), data.heartrate);
      G_->add_or_modify_attrib_local<breathrate_att>(sensor_node.value(), data.breathrate);
      G_->add_or_modify_attrib_local<timestamp_att>(sensor_node.value(), timestamp);
      G_->update_node(sensor_node.value());
      std::cout << "Sensor node [" << sensor_name_ << "] has been updated." << std::endl;
    }
  }
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

  client_.subscribe(topic_, QOS);

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
  if ( (msg->get_topic() == topic_) && (message_type_ == "RespiratoryHeartbeatSensor") ) {
    if (!control_) {return;}
    auto sensor = RespiratoryHeartbeatSensor(json::parse(msg->get_payload_str()));
    sensor_data_to_dsr<RespiratoryHeartbeatSensor>(sensor);
  }
  // TODO: Add new sensor topics and msg types
}

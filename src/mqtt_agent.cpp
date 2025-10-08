// Copyright (c) 2025 Alberto J. Tudela Roldán
// Copyright (c) 2025 José Galeas Merchán
// Copyright (c) 2025 Grupo Avispa, DTE, Universidad de Málaga
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

#include "mqtt_dsr_agent/mqtt_agent.hpp"

MqttAgent::MqttAgent(
  const int & agent_id, const std::string & agent_name,
  const std::string & source, const std::string & topic, 
  const std::string & server_address,
  const std::string & client_id)
: agent_id_(agent_id), agent_name_(agent_name), source_(source), topic_(topic),
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

  // TESTS JP: Crea nodo padre
  auto room_node = G_->get_node("salon");
  if (!room_node.has_value()) {
    room_node.emplace(DSR::Node::create<room_node_type>("salon"));
    if (auto id = G_->insert_node(room_node.value()); id.has_value()) {
      G_->update_node(room_node.value());
    }
  }
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
  auto from_node = G_->get_node(from);
  if (!from_node.has_value()) {
    std::cout << "ERROR: Could not get 'from' node [" << from << "]" << std::endl;
    return;
  }
  auto to_node = G_->get_node(to);
  if (!to_node.has_value()) {
    std::cout << "ERROR: Could not get 'to' node [" << to << "]" << std::endl;
    return;
  }

  if (type == "in") {
    // check if 'from' node is a sensor
    auto sensor_type = G_->get_attrib_by_name<type_of_sensor_att>(from_node.value());
    if (sensor_type.has_value()) {
      // radar respiratory sensor
      if (sensor_type == "datoRadarRespiracion") {
        // check if the 'from' node is a 'person' node
        if (from_node.has_value() && from_node.value().type() == "person") {
          control_ = true;
          person_node_ = from_node;
          // send control msg to sensor
          const char * payload = "OnSensor";
          std::string control_topic = topic_ + "/control";
          std::cout << "Control Topic: " << control_topic << std::endl;
          client_.publish(control_topic, payload, strlen(payload), QOS, false);
          std::cout << "Person: " << person_node_.value().name() << std::endl;
          std::cout << "FMCW Sensor measurement has started...:" << std::endl;
        }
      }
    }
  }
}
// ----------------------------------------------------------------------------------------------

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
  if (edge_tag == "in") {
    std::cout << "Delete edge in between " << from << " and " << to << std::endl;
    auto from_node = G_->get_node(from);
    auto to_node = G_->get_node(to);
    if (from_node.has_value() && person_node_.has_value() && to_node.has_value() &&
       (from_node.value().name() == person_node_.value().name()))
     {
        std::cout << "Delete person in bed" << std::endl;
        person_node_ = {};
        const char * payload = "OffSensor";
        std::string control_topic = topic_ + "/control";
        client_.publish("Sensor/Control", payload, strlen(payload), QOS, false);
        control_ = false;
        std::cout << "Person has left the room, stop measuring ..." << std::endl;
        if (radar_sensor_node_.has_value()) {
          if (G_->delete_edge(radar_sensor_node_.value().id(), person_node_.value().id(), "measuring")) {
            std::cout << "Deleted edge measuring between [" << radar_sensor_node_.value().name() << "] and ["
                      << person_node_.value().name() << "]" << std::endl;
          }
        }
    }
   }
}
// ----------------------------------------------------------------------------------------------

int MqttAgent::sensor_data_to_dsr(json data)
{
  // check all relevant message metadata are available
  if ( (!data.contains("sensorName")) || (!data.contains("sensorType")) || (!data.contains("sensorLocation")) || (!data.contains("parentNode"))) {
    std::cout << "sensor_data_to_dsr ERROR: Received message has not adequate metadata. No action performed" << std::endl;
    return 0;
  }

  string sensor_type_ = data["sensorType"];
  string parent_node_name_ = data["parentNode"];

  // get or create timestamp  
  long long int timestamp_;
  if ( data.contains("timestamp") )
    timestamp_ = data["timestamp"]; // timestamp provided
  else {
    // timestamp assigned upon reception
    std::cout << "WARNING: Received message had no timestamp. Assigning reception time as node timestamp" << std::endl;
    auto now = std::chrono::system_clock::now();
    timestamp_ = static_cast<int>(std::chrono::duration_cast<
        std::chrono::nanoseconds>(now.time_since_epoch()).count());
  }

  // Check if parent node exists (if not, we cannot put the sensor in the world!)
  auto parent_node_ = G_->get_node(parent_node_name_);
  if (!parent_node_.has_value()) {
    std::cout << "ERROR: Could not find parent node [" << parent_node_name_ << "] for sensor [" << sensor_type_ << "]. "
              << "Sensor node not included in the DSR" << std::endl;
    return 0;
  }

  // Then check if sensor node has been created and create it if necessary 
  auto sensor_node = G_->get_node(sensor_type_);
  if (!sensor_node.has_value()) {
    sensor_node.emplace(DSR::Node::create<sensor_node_type>(sensor_type_));
    if (auto id = G_->insert_node(sensor_node.value()); id.has_value()) {
      std::cout << "Inserted sensor node [" << sensor_type_ << "] in the graph."
                << std::endl;
      // Set "IN" edge between room and sensor
      auto edge = DSR::Edge::create<in_edge_type>(sensor_node.value().id(), parent_node_.value().id());
      if (G_->insert_or_assign_edge(edge)) {
        std::cout << "Inserted edge between [" << sensor_type_ << "] and ["
                  << parent_node_name_ << "]" << std::endl;
      }
    }
  }
  
  // insert type_of_sensor attribute
  G_->add_or_modify_attrib_local<type_of_sensor_att>(sensor_node.value(), (std::string)(data["sensorName"]));

  // Check type of msg and update the sensor node with the new data
  if (data["sensorName"] == "ZPHS01B"){
    // Add location (we use a 'room' attribute for this)
    G_->add_or_modify_attrib_local<room_att>(sensor_node.value(), (std::string)(data["sensorLocation"]));
    // Parse air quality values, update the sensor node and insert it
    G_->add_or_modify_attrib_local<pm1_att>(sensor_node.value(), (int)(data["pm1.0 (ug/m3)"]));
    G_->add_or_modify_attrib_local<pm25_att>(sensor_node.value(), (int)(data["pm2.5 (ug/m3)"]));
    G_->add_or_modify_attrib_local<pm10_att>(sensor_node.value(), (int)(data["pm10 (ug/m3)"]));
    G_->add_or_modify_attrib_local<co2_att>(sensor_node.value(), (int)(data["CO2 (ppm)"]));
    G_->add_or_modify_attrib_local<tvoc_att>(sensor_node.value(), (int)(data["TVOC (lvl)"]));
    G_->add_or_modify_attrib_local<ch2o_att>(sensor_node.value(), (int)(data["CH2O (ug/m3)"]));
    G_->add_or_modify_attrib_local<co_att>(sensor_node.value(), (float)(data["CO (ppm)"]));
    G_->add_or_modify_attrib_local<o3_att>(sensor_node.value(), (int)(data["O3 (ppb)"]));
    G_->add_or_modify_attrib_local<no2_att>(sensor_node.value(), (int)(data["NO2 (ppb)"]));
    G_->add_or_modify_attrib_local<temperature_att>(sensor_node.value(), (float)(data["temp (celsius)"]));
    G_->add_or_modify_attrib_local<humidity_att>(sensor_node.value(), (float)(data["humidity (percent)"]));
    if (data.contains("toInfluxDB"))
      G_->add_or_modify_attrib_local<toinflux_att>(sensor_node.value(), (bool)(data["toInfluxDB"]));
    G_->add_or_modify_attrib_local<measure_timestamp_att>(sensor_node.value(), (uint64_t)(timestamp_));
    G_->update_node(sensor_node.value());
    std::cout << "Sensor node [" << sensor_type_ << "] has been updated." << std::endl;
  }  
  else if (data["sensorName"] == "datoRadarRespiracion") {
    // First check data is valid
    if (data["heartrate"] <= 30 || data["breathrate"] <= 10) {
      return 0;
    }  
    // Set "MEASURING" edge between sensor and person once for RespiratoryHeartbeatSensor
    radar_sensor_node_ = sensor_node; 
    auto edges_measuring = G_->get_node_edges_by_type(sensor_node.value(), "measuring");
    if (edges_measuring.empty()) {
      if (person_node_.has_value() && sensor_node.has_value()) {
        auto edge_measure = DSR::Edge::create<measuring_edge_type>(
          sensor_node.value().id(), person_node_.value().id());
        if (G_->insert_or_assign_edge(edge_measure)) {
          std::cout << "Inserted edge between [" << sensor_type_ << "] and ["
                    << person_node_.value().name() << "]" << std::endl;
        }
      }
    }
    // Parse vital parameters, update the sensor node and insert it
    G_->add_or_modify_attrib_local<heartrate_att>(sensor_node.value(), (float)(data["heartrate"]));
    G_->add_or_modify_attrib_local<breathrate_att>(sensor_node.value(), (float)(data["breathrate"]));
    if (data.contains("toInfluxDB"))
      G_->add_or_modify_attrib_local<toinflux_att>(sensor_node.value(), (bool)(data["toInfluxDB"]));
    G_->add_or_modify_attrib_local<measure_timestamp_att>(sensor_node.value(), (uint64_t)(timestamp_));
    G_->update_node(sensor_node.value());
    std::cout << "Sensor node [" << sensor_type_ << "] has been updated." << std::endl;
  }
  else if (data["sensorName"] == "PIR sensor Adafruit ID 189"){
    // Add location (we use a 'room' attribute for this)
    G_->add_or_modify_attrib_local<room_att>(sensor_node.value(), (std::string)(data["sensorLocation"]));
    // Parse presence value, update the sensor node and insert it
    G_->add_or_modify_attrib_local<presence_att>(sensor_node.value(), (bool)(data["presence"]));
    if (data.contains("toInfluxDB"))
      G_->add_or_modify_attrib_local<toinflux_att>(sensor_node.value(), (bool)(data["toInfluxDB"]));
    G_->add_or_modify_attrib_local<measure_timestamp_att>(sensor_node.value(), (uint64_t)(timestamp_));
    G_->update_node(sensor_node.value());
    std::cout << "Sensor node [" << sensor_type_ << "] has been updated." << std::endl;
  } 
  else if (data["sensorName"] == "Grove Sound Sensor"){
    // Add location (we use a 'room' attribute for this)
    G_->add_or_modify_attrib_local<room_att>(sensor_node.value(), (std::string)(data["sensorLocation"]));
    // Parse volume value, update the sensor node and insert it
    G_->add_or_modify_attrib_local<volume_att>(sensor_node.value(), (float)(data["volume"]));
    if (data.contains("toInfluxDB"))
      G_->add_or_modify_attrib_local<toinflux_att>(sensor_node.value(), (bool)(data["toInfluxDB"]));
    G_->add_or_modify_attrib_local<measure_timestamp_att>(sensor_node.value(), (uint64_t)(timestamp_));
    G_->update_node(sensor_node.value());
    std::cout << "Sensor node [" << sensor_type_ << "] has been updated." << std::endl;
  } 
  else {
    std::cout << "sensor_data_to_dsr ERROR: Sensor " << data["sensorName"] << " not supported by mqtt_agent (yet)" << std::endl;
    return 0;
  }

  return 1;
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
  string j_object_str = msg->get_payload_str();
  json j_object = json::parse(j_object_str);

  // check message type
  if ( (!j_object.contains("sensorType")) || (!j_object.contains("sensorName")) ) {
    std::cout << " WARNING: No 'sensorType' or 'sensorName' key found in the message. No action performed" << std::endl;
    return; 
  }
  else {
    std::cout << "Message received from sensor " << j_object["sensorName"] << ". Type: " << j_object["sensorType"] << std::endl;
  }
  if (!sensor_data_to_dsr(j_object))
    std::cout << " WARNING: Data message from sensor " << j_object["sensorName"] << " not uploaded in the DSR" << std::endl;
}

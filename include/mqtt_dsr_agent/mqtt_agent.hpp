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

#ifndef MQTT_AGENT_HPP_
#define MQTT_AGENT_HPP_

#include <iostream>
#include <memory>
#include <string>
#include <vector>
// Qt
#include <QObject>

#include "dsr/api/dsr_api.h"
#include "mqtt/async_client.h"
#include "mqtt_dsr_agent/campero_types.hpp"

/**
 * @class MqttAgent
 * @brief Class to manage the communication between the DSR graph and the MQTT broker
 */
class MqttAgent : public QObject, public virtual mqtt::callback,
  public virtual mqtt::iaction_listener
{
  Q_OBJECT

public:
  /**
   * @brief Construct a new Mqtt Agent object
   *
   * @param config_file path to the config file
   */
  MqttAgent(
    const int & agent_id, const std::string & agent_name,
    const std::string & robot_name, const std::string & topic, const std::string & message_type,
    const std::string & parent_node, const std::string & sensor_name,
    const std::string & server_address,
    const std::string & client_id);

  /**
   * @brief Destructor
   */
  ~MqttAgent();

  /**
   * @brief Set the credentials for the MQTT broker
   *
   * @param username Username
   * @param password Password
   */
  void set_credentials(const std::string & username, const std::string & password);

  /**
   * @brief Connect to the MQTT broker
   *
   * @return bool if the connection is successful
   */
  bool connect();

  /**
   * @brief Disconnect from the MQTT broker
   */
  void disconnect();

  /**
   * @brief Set the topics to subscribe
   *
   * @param topics Topics to subscribe
   */
  // void set_topics(const std::vector<std::string> & topics);

private:
  /* ----------------------------------------  DSR  -------------------- -------------------- */
  // DSR callbacks called when the DSR graph is changed
  void node_updated(std::uint64_t id, const std::string & type);
  void node_attributes_updated(uint64_t id, const std::vector<std::string> & att_names);
  void edge_updated(std::uint64_t from, std::uint64_t to, const std::string & type);
  void edge_attributes_updated(
    std::uint64_t from, std::uint64_t to,
    const std::string & type, const std::vector<std::string> & att_names);
  void node_deleted(std::uint64_t id);
  void edge_deleted(std::uint64_t from, std::uint64_t to, const std::string & edge_tag);
  template<typename T>
  void sensor_data_to_dsr(const T & data);
  /* ----------------------------------------  MQTT  -------------------- -------------------- */

  /**
   * @brief Reconnect to the broker manually by calling connect() again.
   *
   * @param delay Time to wait before reconnecting
   */
  void reconnect(int delay);

  /**
   * @brief Callback for when the reconnection attempt is successful.
   *
   * @param tok Token for the reconnection attempt
   */
  void on_success(const mqtt::token & tok) override;

  /**
   * @brief Callback for when the reconnection attempt fails.
   *
   * @param tok Token for the reconnection attempt
   */
  void on_failure(const mqtt::token & tok) override;

  /**
   * @brief Callback for when the connection is successful.
   *
   * @param cause Cause of the connection
   */
  void connected(const std::string & cause) override;

  /**
   * @brief Callback for when the connection is lost.
   * This will initiate the attempt to manually reconnect.
   *
   * @param cause Cause of the connection loss
   */
  void connection_lost(const std::string & cause) override;

  /**
   * @brief Callback for when a message arrives.
   *
   * @param msg Message received
   */
  void message_arrived(mqtt::const_message_ptr msg) override;

  // DSR graph
  int agent_id_;
  std::string agent_name_;
  std::string robot_name_;
  std::shared_ptr<DSR::DSRGraph> G_;

  // Counter for the number of connection retries
  int nretry_;
  // QoS to subscribe
  const int QOS = 1;
  // Number of connection retries
  const int N_RETRY_ATTEMPTS = 5;
  std::string topic_;
  std::string message_type_;
  std::string parent_node_;
  std::string sensor_name_;
  std::string server_address_;
  std::string client_id_;

  // The MQTT client
  mqtt::async_client client_;
  // Options to use if we need to reconnect
  mqtt::connect_options conn_options_;

  std::optional<DSR::Node> person_node;
  bool control;
};

#endif  // MQTT_AGENT_HPP_

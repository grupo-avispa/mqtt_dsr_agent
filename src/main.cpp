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
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include "mqtt_dsr_agent/mqtt_agent.hpp"

std::map<std::string, std::string> set_configuration(const std::string & config_file)
{
  // Map to be retuned
  std::map<std::string, std::string> config_map;
  // Open config file
  std::ifstream configFile(config_file);
  if (!configFile.is_open()) {
    std::cerr << "Couldn't open config file: " << config_file << std::endl;
    return {};
  }
  // Read lines from config file and parse the parameters
  std::string line;
  std::cout << "Configuration parameters:";
  while (std::getline(configFile, line)) {
    std::istringstream is_line(line);
    std::string key, value;
    if (std::getline(is_line, key, '=') && std::getline(is_line, value)) {
      if (key == "agent_id") {
        config_map["agent_id"] = value;
      } else if (key == "agent_name") {
        config_map["agent_name"] = value;
      } else if (key == "robot_name") {
        config_map["robot_name"] = value;
      } else if (key == "server_address") {
        config_map["server_address"] = value;
      } else if (key == "client_id") {
        config_map["client_id"] = value;
      } else if (key == "topic") {
        config_map["topic"] = value;
      } else if (key == "message_type") {
        config_map["message_type"] = value;
      } else if (key == "parent_node") {
        config_map["parent_node"] = value;
      } else if (key == "sensor_name") {
        config_map["sensor_name"] = value;
      } else {
        std::cerr << "Error parsing not defined parameter: " << key << std::endl;
        return {};
      }
    }
  }
  std::cout << std::endl << " Finished configuration ...";
  configFile.close();
  return config_map;
}

int main(int argc, char * argv[])
{
  QCoreApplication app(argc, argv);
  if (argc != 2) {
    std::cerr << "Error calling the executable, try:" <<
      " <./build/mqtt_dsr_agent path_to_this_directory/etc/config>" << std::endl;
    return -1;
  }
  std::map<std::string, std::string> config_map = set_configuration(argv[1]);
  auto mqtt_agent = std::make_shared<MqttAgent>(
    std::stoi(config_map["agent_id"]),
    config_map["agent_name"],
    config_map["robot_name"],
    config_map["topic"],
    config_map["message_type"],
    config_map["parent_node"],
    config_map["sensor_name"],
    config_map["server_address"],
    config_map["client_id"]);
  mqtt_agent->connect();
  return app.exec();
}

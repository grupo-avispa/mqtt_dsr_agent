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

#include "mqtt_dsr_agent/mqtt_agent.hpp"

const std::string SERVER_ADDRESS("mqtt://192.168.0.140:1883");
const std::string CLIENT_ID("paho_cpp_async_subcribe");
const std::string TOPIC1("cma/person/positional");
const std::string TOPIC2("cma/person/vitals");
const std::string TOPIC3("Sensor/HR");
const std::string TOPIC4("Sensor/BR");

int main(int argc, char * argv[])
{
  QCoreApplication app(argc, argv);

  auto mqtt_agent = std::make_shared<MqttAgent>("mqtt_dsr_agent", 111, SERVER_ADDRESS, CLIENT_ID);
  // mqtt_agent->set_credentials("user", "password");
  mqtt_agent->set_topics({TOPIC1, TOPIC2});
  mqtt_agent->connect();
  return app.exec();
}

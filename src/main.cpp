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

#include "mqtt_dsr_agent/mqtt_agent.hpp"

#include "mqtt/async_client.h"
#include <string>

const std::string SERVER_ADDRESS("mqtt://localhost:1883");
const std::string CLIENT_ID("paho_cpp_async_subcribe");
const std::string TOPIC("hello");

const int	QOS = 1;
const int	N_RETRY_ATTEMPTS = 5;

int main(int argc, char * argv[])
{
  QCoreApplication app(argc, argv);
  auto mqtt_agent = std::make_shared<MqttAgent>("mqtt_agent", 1);
  mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);
  return app.exec();
}

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

int main(int argc, char * argv[])
{
  QCoreApplication app(argc, argv);
  if(argc != 2){
      std::cerr << "Error calling the executable, try:" << 
      " <./build/mqtt_dsr_agent path_to_this_directory/etc/config>" << std::endl;
      return -1;
  }
  auto mqtt_agent = std::make_shared<MqttAgent>(argv[1]);
  mqtt_agent->connect();
  return app.exec();
}

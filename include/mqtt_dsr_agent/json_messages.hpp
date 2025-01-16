// Copyright (c) 2024 Alberto J. Tudela Roldán
// Copyright (c) 2024 José Galeas Merchán
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

#ifndef JSON_MESSAGES
#define JSON_MESSAGES

#include <iostream>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct RespiratoryHeartbeatSensor
{
  float heartrate;
  float breathrate;
  float timestamp;
};

/* Check if is a valid JSON RespiratoryHeartbeatSensor */
inline bool isValidJsonRespiratoryHeartbeatSensor(const json & j)
{
  if (!j.contains("heartrate") || !j.contains("breathrate") ||
    j["heartrate"].get<float>() <= 0 || j["breathrate"].get<float>() <= 0)
  {

    return false;
  }
  return true;
}

/* Convert a RespiratoryHeartbeatSensor struct into a JSON file */
inline void to_json(json & j, const RespiratoryHeartbeatSensor & d)
{
  j["heartrate"] = d.heartrate;
  j["breathrate"] = d.breathrate;
  j["timestamp"] = d.timestamp;
}

/* Convert a JSON file into a RespiratoryHeartbeatSensor struct */
inline void from_json(const json & j, RespiratoryHeartbeatSensor & d)
{
  // Check if is a valid answer
  if (!isValidJsonRespiratoryHeartbeatSensor(j)) {return;}

  j.at("heartrate").get_to(d.heartrate);
  j.at("breathrate").get_to(d.breathrate);
  j.at("timestamp").get_to(d.timestamp);
}


#endif  // JSON_MESSAGES

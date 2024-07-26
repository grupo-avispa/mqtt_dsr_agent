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

#ifndef JSON_MESSAGES
#define JSON_MESSAGES

#include <iostream>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct RespiratoryHeartbeatSensor
{
  int data_size;
  std::vector<float> heartrate;
  std::vector<float> breathrate;
  std::vector<float> timestamp;
};

/* Check if is a valid JSON RespiratoryHeartbeatSensor */
inline bool isValidJsonRespiratoryHeartbeatSensor(const json & j)
{
  if (!j.contains("data_size")) {return false;}

  // Check if the data_size is correct
  int data_size = j["data_size"].get<int>();
  for (int i = 0; i < data_size; i++) {
    std::string sample_id = "sample " + std::to_string(i);
    if (!j.contains(sample_id)) {return false;}
    if (!j[sample_id].contains("heartrate")) {return false;}
    if (!j[sample_id].contains("breathrate")) {return false;}
    if (!j[sample_id].contains("timestamp")) {return false;}
  }
  return true;
}

/* Convert a RespiratoryHeartbeatSensor struct into a JSON file */
inline void to_json(json & j, const RespiratoryHeartbeatSensor & d)
{
  j["data_size"] = d.data_size;
  for (int i = 0; i < d.data_size; i++) {
    std::string sample_id = "sample " + std::to_string(i);
    j[sample_id]["heartrate"] = d.heartrate[i];
    j[sample_id]["breathrate"] = d.breathrate[i];
    j[sample_id]["timestamp"] = d.timestamp[i];
  }
}

/* Convert a JSON file into a RespiratoryHeartbeatSensor struct */
inline void from_json(const json & j, RespiratoryHeartbeatSensor & d)
{
  // Check if is a valid answer
  if (!isValidJsonRespiratoryHeartbeatSensor(j)) {return;}

  // Fill the RespiratoryHeartbeatSensor struct
  j.at("data_size").get_to(d.data_size);
  d.heartrate.resize(d.data_size);
  d.breathrate.resize(d.data_size);
  d.timestamp.resize(d.data_size);
  for (int i = 0; i < d.data_size; i++) {
    std::string sample_id = "sample " + std::to_string(i);
    j[sample_id].at("heartrate").get_to(d.heartrate[i]);
    j[sample_id].at("breathrate").get_to(d.breathrate[i]);
    j[sample_id].at("timestamp").get_to(d.timestamp[i]);
  }
}


#endif  // JSON_MESSAGES

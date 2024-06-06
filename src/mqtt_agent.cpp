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

MqttAgent::MqttAgent(std::string agent_name, int agent_id)
: agent_name_(agent_name), agent_id_(agent_id)
{
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
}

MqttAgent::~MqttAgent()
{
  G_.reset();
}

// Callbacks called when the DSR graph is changed
void MqttAgent::node_updated(std::uint64_t /*id*/, const std::string & /*type*/)
{

}

void MqttAgent::node_attributes_updated(uint64_t /*id*/, const std::vector<std::string> & /*att_names*/)
{

}

void MqttAgent::edge_updated(std::uint64_t /*from*/, std::uint64_t /*to*/, const std::string & /*type*/)
{

}

void MqttAgent::edge_attributes_updated(
  std::uint64_t /*from*/, std::uint64_t /*to*/,
  const std::string & /*type*/, const std::vector<std::string> & /*att_names*/)
{

}
void MqttAgent::node_deleted(std::uint64_t /*id*/)
{

}
void MqttAgent::edge_deleted(std::uint64_t /*from*/, std::uint64_t /*to*/, const std::string & /*edge_tag*/)
{

}

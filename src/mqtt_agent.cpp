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

// Functions to 'send' data to the DSR graph
template <typename node_type>
void MqttAgent::insert_node(const std::string &name)
{
  auto new_node = DSR::Node::create<node_type>(name);
  if (auto id = G_->insert_node(new_node); id.has_value()){
    std::cout << "Node " << name << " inserted with id " << id.value() << std::endl;
  }
}

template <typename edge_type>
void MqttAgent::insert_edge(const std::string &from, const std::string &to)
{
  // Get the relatives nodes
  auto parent_node = G_->get_node(from);
  auto child_node = G_->get_node(to);
  if (parent_node.has_value()){
    if (child_node.has_value()){
      auto new_edge = DSR::Edge::create<edge_type>(parent_node.value().id(), child_node.value().id());
      if (G_->insert_or_assign_edge(new_edge)){
        std::cout << "Edge from " << from << " to " << to << " inserted" << std::endl;
      }
    }
  }
}

template <typename att_type, typename value_type>
void MqttAgent::insert_attribute(const std::string & node, const value_type & att_value)
{
  if (auto new_node = G_->get_node(node); new_node.has_value()){
    G_->add_or_modify_attrib_local<att_type>(new_node.value(), att_value);
    G_->update_node(new_node.value());
    std::cout << "Attribute " << att_value << " inserted in node " << node << std::endl;
  }
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

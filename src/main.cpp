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

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include "mqtt/async_client.h"
#include <string>

#include "json.hpp"
#include "json_fwd.hpp"

using json = nlohmann::json;	

const std::string SERVER_ADDRESS("mqtt://192.168.0.140:1883");
const std::string CLIENT_ID("paho_cpp_async_subcribe");
const std::string TOPIC1("cma/person/positional");
const std::string TOPIC2("cma/person/vitals");
const std::string TOPIC3("Sensor/HR");
const std::string TOPIC4("Sensor/BR");

const int	QOS = 1;
const int	N_RETRY_ATTEMPTS = 5;

auto mqtt_agent = std::make_shared<MqttAgent>("mqtt_agent", 111);

/////////////////////////////////////////////////////////////////////////////

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

class action_listener : public virtual mqtt::iaction_listener
{
	std::string name_;

	void on_failure(const mqtt::token& tok) override {
		std::cout << name_ << " failure";
		if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		std::cout << std::endl;
	}

	void on_success(const mqtt::token& tok) override {
		std::cout << name_ << " success";
		if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		auto top = tok.get_topics();
		if (top && !top->empty())
			std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
		std::cout << std::endl;
	}

public:
	action_listener(const std::string& name) : name_(name) {}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */
class callback : public virtual mqtt::callback,
					public virtual mqtt::iaction_listener

{
	// Counter for the number of connection retries
	int nretry_;
	// The MQTT client
	mqtt::async_client& cli_;
	// Options to use if we need to reconnect
	mqtt::connect_options& connOpts_;
	// An action listener to display the result of actions.
	action_listener subListener_;

	// This deomonstrates manually reconnecting to the broker by calling
	// connect() again. This is a possibility for an application that keeps
	// a copy of it's original connect_options, or if the app wants to
	// reconnect with different options.
	// Another way this can be done manually, if using the same options, is
	// to just call the async_client::reconnect() method.
	void reconnect() {
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		try {
			cli_.connect(connOpts_, nullptr, *this);
		}
		catch (const mqtt::exception& exc) {
			std::cerr << "Error: " << exc.what() << std::endl;
			exit(1);
		}
	}

	// Re-connection failure
	void on_failure(const mqtt::token& tok) override {
		std::cout << "Connection attempt failed" << std::endl;
    if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		std::cout << std::endl;
		if (++nretry_ > N_RETRY_ATTEMPTS)
			exit(1);
		reconnect();
	}

	// (Re)connection success
	// Either this or connected() can be used for callbacks.
	void on_success(const mqtt::token& tok) override {
		std::cout << "Connection Success" << std::endl;
    if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		std::cout << std::endl;
  }

	// (Re)connection success
	void connected(const std::string& cause) override {
    if (!cause.empty())
			std::cout << "\tcause: " << cause << std::endl;
		std::cout << "\nConnection success" << std::endl;
		std::cout << "\nSubscribing to topics '" << TOPIC1 << "', \t'"
			<< TOPIC2 << "'\n"
			<< "\tfor client " << CLIENT_ID
			<< " using QoS" << QOS << "\n"
			<< "\nPress Q<Enter> to quit\n" << std::endl;

		cli_.subscribe(TOPIC1, QOS, nullptr, subListener_);
		cli_.subscribe(TOPIC2, QOS, nullptr, subListener_);
	}

	// Callback for when the connection is lost.
	// This will initiate the attempt to manually reconnect.
	void connection_lost(const std::string& cause) override {
		std::cout << "\nConnection lost" << std::endl;
		if (!cause.empty())
			std::cout << "\tcause: " << cause << std::endl;

		std::cout << "Reconnecting..." << std::endl;
		nretry_ = 0;
		reconnect();
	}
	/*
	float media = 0.0f;
	unsigned int valores = 0; 
	*/

	// Callback for when a message arrives.
	void message_arrived(mqtt::const_message_ptr msg) override {

		/*
		std::cout << "Message arrived" << "'\n";
		std::cout << "\ttopic: '" << msg->get_topic() << "'\n";
		std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
		*/

		// media de distancia
		/*
		if (msg->get_topic() == "person/distance") {
			json j = json::parse(msg->get_payload_str());
			float distancia = j["distance"].get<float>(); 
			if (distancia != -1) {
				media = (media*valores + distancia)/(valores+1);
				valores++;
				std::cout << "Distancia media: '" << media << "'\n" << std::endl;
			}
			media = (valores > 20) ? 0:media;
			valores = (valores > 20) ? 0:valores;
		}
		*/

		if (msg->get_topic() == "cma/person/positional") {
			json j = json::parse(msg->get_payload_str());
			float distancia = j["distance"].get<float>();
			int time_stamp = j["timestamp"].get<int>();
			std::cout << "Detected person\n"
				<< "At program time: " << time_stamp << "ms \n" 
				<< "At: " << distancia << "m" << std::endl;
			// Sensor colocado en el baño --> Detecta persona [person in bathroom]
			if(mqtt_agent->person_node.has_value() && mqtt_agent->control){ // persona ya está en el baño
				mqtt_agent->insert_attribute<distancia_att, float>(mqtt_agent->person_node.value().name(), distancia);
				mqtt_agent->insert_attribute<distanciaTime_att, int>(mqtt_agent->person_node.value().name(), time_stamp);
			} else { // Insertamos persona en el baño
				mqtt_agent->insert_node<person_node_type>("person");
				mqtt_agent->insert_edge<in_edge_type>("person", "bathroom");
				mqtt_agent->insert_attribute<distancia_att, float>("person", distancia);
				mqtt_agent->insert_attribute<distanciaTime_att, int>("person", time_stamp);
			}
			

			// Insertar las cosas de los nodos cuando hay una persona interacting
			//if(mqtt_agent->person_node.has_value() && mqtt_agent->control){
				//std::cout << "Insert attribute to person: " << mqtt_agent->person_node.value().name() << std::endl;
				// mqtt_agent->insert_attribute<distancia_att, float>(mqtt_agent->person_node.value().name(), distancia);
				// mqtt_agent->insert_attribute<distanciaTime_att, int>(mqtt_agent->person_node.value().name(), time_stamp);
			//}
		}
		
		if (msg->get_topic() == "cma/person/vitals") {
			json j = json::parse(msg->get_payload_str());
			int numero_datos = j["data_size"].get<int>();
			std::cout << "Vital sings red." 
				<< numero_datos << " Samples Taken" << std::endl;
			for (int i = 0; i < numero_datos; i++) {
				std::string sample_id = "sample "+std::to_string(i);
				const auto& sample = j[sample_id];
				float timestamp = sample["timestamp"].get<int>();
				float heart = sample["heartrate"].get<float>();
				float breath = sample["breathrate"].get<float>();
				std::cout << "Sample " << i << "\n" 
					<< "Time " << timestamp << "\n"
					<< "Heart Rate " << heart << "\n"
					<< "Breath Rate " << breath << std::endl;
				
				// Insertar las cosas de los nodos cuando hay una persona interacting
				if(mqtt_agent->person_node.has_value() && mqtt_agent->control){
					// mqtt_agent->insert_attribute<heartRate_att, float>(mqtt_agent->person_node.value().name(), heart);
					// mqtt_agent->insert_attribute<breathRate_att, float>(mqtt_agent->person_node.value().name(), breath);
					// mqtt_agent->insert_attribute<vitalsTime_att, int>(mqtt_agent->person_node.value().name(), timestamp);
				}
			}  
		}

		if (msg->get_topic() == "Sensor/HR") {
			if(mqtt_agent->person_node.has_value() && mqtt_agent->control){
				std::string hr = msg->get_payload_str();
				mqtt_agent->insert_attribute<heartRate2_att, std::string>(mqtt_agent->person_node.value().name(), hr);
				//mqtt_agent->insert_attribute<breathRate_att, float>(mqtt_agent->person_node.value().name(), breath);
			}
		}
	}
	// void delivery_complete(mqtt::delivery_token_ptr token) override {
  //   if (token.msg)
	// 		std::cout << "\tcause: " << cause << std::endl;
  // }

public:
	callback(mqtt::async_client& cli, mqtt::connect_options& connOpts)
				: nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}
};

/////////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[])
{

  QCoreApplication app(argc, argv);
  std::cout << "Initialize graph" << std::endl;


//   mqtt_agent->insert_node<carlos_sensor_node_type>("sensor_carlos");
//   mqtt_agent->insert_edge<has_edge_type>("sensor_carlos", "robot");
//   mqtt_agent->insert_attribute<distancia_att, float>("sensor_carlos", 0.0f);
//   mqtt_agent->insert_attribute<distanciaTime_att, int>("sensor_carlos", 0.0f);
//   mqtt_agent->insert_attribute<heartRate_att, float>("sensor_carlos", 0.0f);
//   mqtt_agent->insert_attribute<breathRate_att, float>("sensor_carlos", 0.0f);
//   mqtt_agent->insert_attribute<vitalsTime_att, int>("sensor_carlos", 0.0f);

	// A subscriber often wants the server to remember its messages when its
	// disconnected. In that case, it needs a unique ClientID and a
	// non-clean session.

  std::cout << "Graph initialized" << std::endl;

	mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

	mqtt::connect_options connOpts;
	connOpts.set_clean_session(false);

	// Install the callback(s) before connecting.
	callback cb(cli, connOpts);
	cli.set_callback(cb);

	/*
	mqtt_agent->insert_attribute<distancia_att_type, float>("CMA_Sesor", "float");
*/
	// Start the connection.
	// When completed, the callback will subscribe to topic.

	try {
		std::cout << "Connecting to the MQTT server..." << std::flush;
		cli.connect(connOpts, nullptr, cb);
	}
	catch (const mqtt::exception& exc) {
		std::cerr << "\nERROR: Unable to connect to MQTT server: '"
			<< SERVER_ADDRESS << "'" << exc << std::endl;
		return 1;
	}

	// // Disconnect

	// try {
	// 	std::cout << "\nDisconnecting from the MQTT server..." << std::flush;
	// 	cli.disconnect()->wait();
	// 	std::cout << "OK" << std::endl;
	// }
	// catch (const mqtt::exception& exc) {
	// 	std::cerr << exc << std::endl;
	// 	return 1;
	// }

 	return app.exec();
}
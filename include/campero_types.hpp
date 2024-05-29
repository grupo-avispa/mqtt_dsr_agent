
#ifndef CAMPERO_TYPES_HPP_
#define CAMPERO_TYPES_HPP_

REGISTER_NODE_TYPE(brain)
REGISTER_NODE_TYPE(navigation)
REGISTER_NODE_TYPE(move)
REGISTER_NODE_TYPE(say)
REGISTER_NODE_TYPE(play)
REGISTER_NODE_TYPE(use_case)
REGISTER_NODE_TYPE(set_volume)
REGISTER_NODE_TYPE(show)
REGISTER_NODE_TYPE(update_bbdd)
REGISTER_NODE_TYPE(tracking)
REGISTER_NODE_TYPE(bring_water)
REGISTER_NODE_TYPE(person)
REGISTER_NODE_TYPE(sensor)

// WORLD SENSOR TYPES
REGISTER_EDGE_TYPE(stopped)
REGISTER_EDGE_TYPE(is)
REGISTER_EDGE_TYPE(is_performing)
REGISTER_EDGE_TYPE(is_with)
REGISTER_EDGE_TYPE(wants_to)
REGISTER_EDGE_TYPE(finished)
REGISTER_EDGE_TYPE(abort)
REGISTER_EDGE_TYPE(aborting)
REGISTER_EDGE_TYPE(cancel)
REGISTER_EDGE_TYPE(failed)
REGISTER_EDGE_TYPE(navigating)

// General types
REGISTER_TYPE(priority, int, false)
REGISTER_TYPE(result_code, std::string, false)
REGISTER_TYPE(number, int, false)
REGISTER_TYPE(source, std::string, false)
REGISTER_TYPE(timestamp, int, false)

// Navigation types
REGISTER_TYPE(pose_x, float, false)
REGISTER_TYPE(pose_y, float, false)
REGISTER_TYPE(pose_angle, float, false)
REGISTER_TYPE(goal_x, float, false)
REGISTER_TYPE(goal_y, float, false)
REGISTER_TYPE(goal_angle, float, false)
REGISTER_TYPE(zone, std::string, false)
REGISTER_TYPE(zones, std::string, false)

// Play / say types
REGISTER_TYPE(text, std::string, false)
REGISTER_TYPE(sound, std::string, false)
REGISTER_TYPE(volume, float, false)

// Show types
REGISTER_TYPE(interface, std::string, false)

// Battery types
REGISTER_TYPE(battery_percentage, float, false)
REGISTER_TYPE(battery_power_supply_status, std::string, false)

// Use case types: do_nothing, wandering, charging, menu, music, neuron_up, getme, reminder, announcer
REGISTER_TYPE(use_case_id, std::string, false)
REGISTER_TYPE(menu_choices, std::string, false)

// Person types
REGISTER_TYPE(identifier, std::string, false)
REGISTER_TYPE(safe_distance, float, false)
REGISTER_TYPE(comm_parameters, std::string, false)
REGISTER_TYPE(skills_parameters, std::string, false)
REGISTER_TYPE(menu, std::string, false)
REGISTER_TYPE(reminder, bool, false)
REGISTER_TYPE(activities, std::string, false)
REGISTER_TYPE(tracking_enable, bool, false)
REGISTER_TYPE(neuron, bool, false)
REGISTER_TYPE(accuracy, float, false)
REGISTER_TYPE(distance, float, false)
REGISTER_TYPE(media, float, false)

// BBDD types
REGISTER_TYPE(bbdd_agent, std::string, false)
REGISTER_TYPE(changes, std::string, false)

// SENSOR types
REGISTER_TYPE(temperature, float, false); // (name,type,false)
REGISTER_TYPE(media, float, false)
REGISTER_TYPE(distance, float, false)

#endif // CAMPERO_TYPES_HPP_

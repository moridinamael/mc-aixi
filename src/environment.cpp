#include <cassert>
#include <sstream>

#include "environment.hpp"
#include "util.hpp"

using namespace std;

std::string Environment::print() const {
	std::ostringstream out;
	out << "action = " << m_action << ", observation = "
		<< m_observation << ", reward = " << m_reward << std::endl;
	return out.str();
}

// By default, check that the action is between the min and max action.
bool Environment::isValidAction(const action_t action) const {
	return minAction() <= action && action <= maxAction();
}

// By default, check that the observation is between the min and max observation.
bool Environment::isValidObservation(const percept_t observation) const {
	return minObservation() <= observation && observation <= maxObservation();
}

// // By default, check that the reward is between the min and max reward.
bool Environment::isValidReward(const percept_t reward) const {
	return minReward() <= reward && reward <= maxReward();
}


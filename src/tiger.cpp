#include <cassert>
#include <sstream>
#include "util.hpp"
#include "tiger.hpp"

// Actions
const action_t Tiger::aListen = 0;
const action_t Tiger::aLeft = 1;
const action_t Tiger::aRight = 2;

// Observations
const percept_t Tiger::oNull = 0;
const percept_t Tiger::oLeft = 1;
const percept_t Tiger::oRight = 2;

// Rewards
const percept_t Tiger::rEaten = 0;   // -100
const percept_t Tiger::rListen = 99; // -1
const percept_t Tiger::rGold = 110;  // +10

const double Tiger::cDefaultListenAccuracy = 0.85;

// Initialise environment
Tiger::Tiger(options_t &options) {
	// Set the accuracy of the listen action
	getOption(options, "tiger-listen-accuracy",
	           cDefaultListenAccuracy, m_listen_accuracy);

	placeTiger();
	m_observation = oNull;
	m_reward = 0;
}

// Randomly place tiger behind one door and gold behind the other.
void Tiger::placeTiger() {
	m_tiger = (rand01() < 0.5 ? oLeft : oRight);
	m_gold = (m_tiger == oLeft ? oRight : oLeft);
}

// Execute an action
void Tiger::performAction(const action_t action) {
	assert(isValidAction(action));
	m_action = action;

	if (action == aListen) {
		// Listen for the tiger, return the correct door with probability
		// equal to m_listen_accuracy.
		m_reward = rListen;
		m_observation = (rand01() < m_listen_accuracy ? m_tiger : m_gold);
	} else {
		// Open a door. Set reward according to what we find. Set observation to
		// null. Replace tiger and gold.
		if (action == aLeft) {
			m_reward = (m_tiger == oLeft ? rEaten : rGold);
		} else if (action == aRight) {
			m_reward = (m_tiger == oRight? rEaten : rGold);
		}
		m_observation = oNull;
		placeTiger();
	}
}

// Print a representation of the environment.
std::string Tiger::print() const {
	std::ostringstream out;

	// Print action
	out << "action = ";
	if(m_action == aListen) out << "listen";
	else if(m_action == aLeft) out << "open left door";
	else if(m_action == aRight) out << "open right door";

	// Print observation
	out << ", observation = ";
	if(m_observation == oNull) out << "null";
	else if(m_observation == oLeft) out << "hear tiger at left door";
	else if(m_observation == oRight) out << "hear tiger at right door";

	// Print reward
	out << ", reward = ";
	if(m_reward == rEaten) out << "eaten";
	else if(m_reward == rListen) out << "listen";
	else if(m_reward == rGold) out << "gold!";
	out << " (" << m_reward << ")";

	out << std::endl;
	return out.str();
}


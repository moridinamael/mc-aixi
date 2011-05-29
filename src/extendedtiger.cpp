#include <cassert>
#include "extendedtiger.hpp"
#include "util.hpp"

// Actions
const action_t ExtendedTiger::aListen = 0;
const action_t ExtendedTiger::aLeft = 1;
const action_t ExtendedTiger::aRight = 2;
const action_t ExtendedTiger::aStand = 3;

// Observations
const percept_t ExtendedTiger::oNull = 0;
const percept_t ExtendedTiger::oLeft = 1;
const percept_t ExtendedTiger::oRight = 2;

// Rewards
const percept_t ExtendedTiger::rInvalid = 0;
const percept_t ExtendedTiger::rTiger = 0;
const percept_t ExtendedTiger::rStand = 99;
const percept_t ExtendedTiger::rListen = 100;
const percept_t ExtendedTiger::rGold = 130;

const double ExtendedTiger::cDefaultListenAccuracy = 0.85;

// Configure environment.
ExtendedTiger::ExtendedTiger(options_t &options) {
	// Set the accuracy of the listen action
	getOption(options, "tiger-listen-accuracy",
	          cDefaultListenAccuracy, m_listen_accuracy);
	assert(0 <= m_listen_accuracy && m_listen_accuracy <= 1.0);

	// Initialise percept and environment state
	m_reward = 0;
	m_observation = oNull;
	reset();
}

// Randomly place tiger behind one door and gold behind the other. Reseat agent.
void ExtendedTiger::reset() {
	m_tiger = rand01() < 0.5 ? oLeft : oRight;  // Randomly place tiger
	m_gold = m_tiger == oLeft ? oRight : oLeft; // Opposite to tiger
	m_sitting = true;                           // Re-seat agent
}

// Execute agent's action.
void ExtendedTiger::performAction(const action_t action) {
	assert(isValidAction(action));
	m_action = action;

	// Unless explicitly accounted for, action is invalid.
	m_observation = oNull; // Some valid actions also have null observation.
	m_reward = rInvalid;

	if (action == aListen && m_sitting) {
		// Listen while sitting down. Observe door hiding tiger with probability
		// m_listen_accuracy otherwise observe other door.
		m_observation = rand01() < m_listen_accuracy ? m_tiger : m_gold;
		m_reward = rListen;
	} else if (action == aLeft && !m_sitting) {
		// Open left door while standing. Get reward based on what was behind
		// the door. Reseat agent and reallocate tiger and gold.
		m_reward = (m_tiger == oLeft ? rTiger : rGold);
		reset();
	} else if (action == aRight && !m_sitting) {
		// Open right door while standing. Get reward based on what was behind
		// the door. Reseat agent and reallocate tiger and gold.
		m_reward = (m_tiger == oRight ? rTiger : rGold);
		reset();
	} else if (action == aStand && m_sitting) {
		// Stand from a sitting position. Get reward for standing.
		m_reward = rStand;
		m_sitting = false;
	}
}

// Print environment state
std::string ExtendedTiger::print() const {
	std::ostringstream out;

	// Print action
	out << "action = ";
	if(m_action == aListen)     out << "listen";
	else if(m_action == aLeft)  out << "open left door";
	else if(m_action == aRight) out << "open right door";
	else if(m_action == aStand) out << "stand up";

	// Print observation
	out << ", observation = ";
	if(m_observation == oNull)       out << "null";
	else if(m_observation == oLeft)  out << "hear tiger at left door";
	else if(m_observation == oRight) out << "hear tiger at right door";

	// Print reward
	out << ", reward = ";
	if(m_reward == rTiger)        out << "eaten";
	else if(m_reward == rInvalid) out << "invalid action";
	else if(m_reward == rStand)   out << "stand up";
	else if(m_reward == rListen)  out << "listen";
	else if(m_reward == rGold)    out << "gold!";
	out << " (" << m_reward << ")";

	out << ", agent is now " << (m_sitting ? "sitting" : "standing")
	    << std::endl;
	return out.str();
}

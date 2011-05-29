#include <cassert>
#include <sstream>
#include "rock-paper-scissors.hpp"
#include "util.hpp"

// Actions
const action_t RockPaperScissors::aRock = 0;
const action_t RockPaperScissors::aPaper = 1;
const action_t RockPaperScissors::aScissors = 2;

// Observations
const percept_t RockPaperScissors::oRock = 0;
const percept_t RockPaperScissors::oPaper = 1;
const percept_t RockPaperScissors::oScissors = 2;

// Rewards
const percept_t RockPaperScissors::rLose = 0;
const percept_t RockPaperScissors::rDraw = 1;
const percept_t RockPaperScissors::rWin = 2;

RockPaperScissors::RockPaperScissors(options_t &options){
	m_reward = 0;
	m_observation = oPaper; // i.e. not rock (to ensure random choice on first action)
}

void RockPaperScissors::performAction(const action_t action){
	assert(isValidAction(action));
	m_action = action;

	// Opponent plays rock if it won the last round by playing rock, otherwise
	// it plays randomly.
	if(m_observation == aRock && m_reward == rLose) {
		m_observation = aRock;
	} else {
		m_observation = randRange(3);
	}

	// Determine reward (0 = player lost, 1 = draw, 2 = player won)
	if (action == m_observation) {
		m_reward = rDraw; // draw
	} else if(action == aRock) {
		m_reward = (m_observation == oScissors ? rWin : rLose);
	} else if(action == aScissors) {
		m_reward = (m_observation == oPaper ? rWin : rLose);
	} else if(action == aPaper) {
		m_reward = (m_observation == oRock ? rWin : rLose);
	}
}

std::string RockPaperScissors::print() const {
	std::ostringstream out;
	out << "Agent played "
	    << (m_action == aRock ? "rock" :
	    	(m_action == aPaper ? "paper" : "scissors"))
	    << ", enivornment played "
	    << (m_observation == oRock ? "rock" :
	    	(m_observation == oPaper ? "paper" : "scissors"))
	    << std::endl << "\tAgent "
	    << (m_reward == rWin ? "wins" :
	    	(m_reward == rDraw ? "draws" : "loses"))
	    << std::endl;
	return out.str();
}

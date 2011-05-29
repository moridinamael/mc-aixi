#include <cassert>
#include "coinflip.hpp"

// Actions
const action_t CoinFlip::aTails = 0;
const action_t CoinFlip::aHeads = 1;

// Observations
const percept_t CoinFlip::oTails = 0;
const percept_t CoinFlip::oHeads = 1;

// Rewards
const percept_t CoinFlip::rLoss = 0;
const percept_t CoinFlip::rWin = 1;

// Constants
const double CoinFlip::cDefaultProbability = 0.7;

CoinFlip::CoinFlip(options_t &options) {
	// Determine the probability of the coin landing on heads
	getOption(options, "coin-flip-p", cDefaultProbability, m_probability);
	assert(0.0 <= m_probability && m_probability <= 1.0);

	// Initial percept
	m_observation = rand01() < m_probability ? 1 : 0;
	m_reward = 0;
}

void CoinFlip::performAction(const action_t action) {
	assert(isValidAction(action));
	m_action = action; // Save action

	// Flip coin, set observation and reward appropriately.
	if(rand01() < m_probability) {
		m_observation = oHeads;
		m_reward = (action == oHeads ? rWin : rLoss);
	} else {
		m_observation = oTails;
		m_reward = (action == oTails ? rWin : rLoss);
	}
}

std::string CoinFlip::print() const {
	std::ostringstream out;
	out << "prediction: " << (m_action == aTails ? "tails" : "heads")
		<< ", observation: " << (m_observation == oTails ? "tails" : "heads")
		<< ", reward: " << m_reward << std::endl;
	return out.str();
}

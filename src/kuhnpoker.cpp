#include <cassert>
#include "kuhnpoker.hpp"
#include "util.hpp"

// Actions
const action_t KuhnPoker::aBet = 0;
const action_t KuhnPoker::aPass = 1;

// Observations
// Final observation is of the form "agent-card + opponent-bet-status"
const percept_t KuhnPoker::oJack = 0;
const percept_t KuhnPoker::oQueen = 1;
const percept_t KuhnPoker::oKing = 2;
const percept_t KuhnPoker::oBet = 0;
const percept_t KuhnPoker::oPass = 4;

// Rewards
const percept_t KuhnPoker::rBetLoss = 0;
const percept_t KuhnPoker::rPassLoss = 1;
const percept_t KuhnPoker::rPassWin = 3;
const percept_t KuhnPoker::rBetWin = 4;

// Betting constants
const double KuhnPoker::cBetProbKing = 0.7;
const double KuhnPoker::cBetProbQueen = (1.0 + KuhnPoker::cBetProbKing) / 3.0;
const double KuhnPoker::cBetProbJack = KuhnPoker::cBetProbKing / 3.0;

KuhnPoker::KuhnPoker(options_t &options) {
	m_reward = 0;
	reset();
}

// Select a card uniformly at random.
percept_t KuhnPoker::randomCard() const {
	const int r = randRange(3);
	return (r == 0 ? oJack : (r == 1 ? oQueen : oKing));
}

// Begin a new round: save previous actions/cards, deal new cards, choose
// environment's (opponent's) first action, compute observation.
void KuhnPoker::reset() {
	// Save the previous actions/cards for use by print()
	m_env_previous_action = m_env_action;
	m_agent_previous_card = m_agent_card;
	m_env_previous_card = m_env_card;

	// Deal cards
	m_agent_card = randomCard();
	m_env_card = m_agent_card;
	while(m_env_card == m_agent_card)
		m_env_card = randomCard();

	// Choose the environment's first action. Bet with a certain probability
	// on jack and king, pass on queen.
	if(m_env_card == oJack)
		m_env_action = rand01() < cBetProbJack ? aBet : aPass;
	else if(m_env_card == oQueen)
		m_env_action = aPass; // Always pass on queen
	else if(m_env_card == oKing)
		m_env_action = rand01() < cBetProbKing ? aBet : aPass;

	// Compute observation: agent-card + environment-bet-status
	m_observation = m_agent_card + (m_env_action == aPass ? oPass : oBet);
}


void KuhnPoker::performAction(action_t action) {
	assert(isValidAction(action));
	m_action = action;

	// If the agent did not call the environments bet then the agent loses
	if(m_action == aPass && m_env_action == aBet) {
		m_reward = rPassLoss;
		reset();
		return;
	}

	// If the environment passed and the agent bet, then the environment has
	// a chance to change its mind.
	if(m_action == aBet && m_env_action == aPass) {
		if(m_env_card == oQueen && rand01() < cBetProbQueen) {
			m_env_action = aBet; // Bet with cBetProbQueen probability on queen
		} else if(m_env_card == oKing) {
			m_env_action = aBet; // Always bet on king
		} else {
			// Environment continues to pass, so agent wins
			m_reward = rPassWin;
			reset();
			return;
		}
	}

	// Players have bet the same amount, winner has highest card
	bool agent_wins = m_env_card == oJack
		|| (m_env_card == oQueen && m_agent_card == oKing);
	if(agent_wins)
		m_reward = (m_env_action == aBet ? rBetWin : rPassWin);
	else
		m_reward = (m_action == aBet ? rBetLoss : rPassLoss);
	reset();
}

// Return a string describing a card
std::string KuhnPoker::cardToString(const percept_t card) const {
	if(card == oJack)       return std::string("jack");
	else if(card == oQueen) return std::string("queen");
	else if(card == oKing)  return std::string("king");
}

// Print the state of the environment
std::string KuhnPoker::print() const {
	std::ostringstream out;

	// Cards and bets
	out << "agent card = " << cardToString(m_agent_previous_card)
		<< ", environment card = " << cardToString(m_env_previous_card)
	    << ", agent " << (m_action == aPass ? "passes" : "bets")
	    << ", environment "
	    << (m_env_previous_action == aPass ? "passes" : "bets") << std::endl;

	// Winner and reward
	bool agent_wins = (m_reward == rPassWin || m_reward == rBetWin);
	out << "agent " << (agent_wins ? "wins" : "loses") << ", reward = "
		<< m_reward << std::endl;

	return out.str();
}

#ifndef __KUHNPOKER_HPP__
#define __KUHNPOKER_HPP__
#include <string>
#include "environment.hpp"


/** Kuhn Poker is a simplified, zero-sum, two player poker variant that uses a
 * deck of three cards: a King, Queen and Jack. Whilst considerably less
 *  sophisticated than popular poker variants such as Texas Hold'em, well-known
 *  strategic concepts such as bluffing and slow-playing remain characteristic
 *  of strong play.
 *
 * In this setup, the agent acts second in a series of rounds. Two actions, pass
 * (KuhnPoker::aPass) or bet (KuhnPoker::aBet), are available to each player.
 * A bet action requires the player to put an extra chip into play. At the
 * beginning of each round, each player puts a chip into play. The environment
 * (opponent) then decides whether to pass or bet; betting will win the round
 * if the agent subsequently passes, otherwise a showdown will occur. In a
 * showdown, the player with the highest card wins the round (i.e. King beats
 * Queen, Queen beats Jack). If the environment (opponent) passes, the agent
 * can either bet or pass; passing leads immediately to a showdown, whilst
 * betting requires the environment (opponent) to either bet to force a
 * showdown, or to pass and let the agent win the round uncontested. The winner
 * of the round gains a reward equal to the total chips in play
 * (KuhnPoker::rPassWin or KuhnPoker::rBetWin), the loser receives a penalty
 * equal to the number of chips they put into play this round
 * (KuhnPoker::rPassLoss or KuhnPoker::rBetLoss). At the end of the round, all
 * chips are removed from play and another round begins.
 *
 * Domain characteristics:
 * - environment: "kuhnpoker"
 * - maximum action: 1 (1 bit)
 * - maximum observation: 6 (3 bits)
 * - maximum reward: 4 (3 bits) */
class KuhnPoker: public Environment {
public:
	
	//set up the initial environment percept
	KuhnPoker(options_t &options);
	
	//Receives the agent's action and calculates the new environment percept
	virtual void performAction(action_t action);
	
	virtual action_t maxAction() const { return 1; }
	virtual percept_t maxObservation() const { return 6; }
	virtual percept_t maxReward() const { return 4; }

	virtual std::string print() const;

private:
	/** Begin a new round. Save necessary information for KuhnPoker::print(),
	 * deal new cards, choose environment's (opponent's) first action and
	 * compute initial observation. */
	void reset();

	/** Choose a card uniformly at random. */
	percept_t randomCard() const;

	/** Turn a card observation into a human-readable string. */
	std::string cardToString(const percept_t card) const;

	/** The enviroment's (opponent's) current action. */
	action_t m_env_action;

	/** The agent's card. */
	percept_t m_agent_card;

	/** The environment's (opponent's) card. */
	percept_t m_env_card;

	/** The agent's card in the previous round. Used by KuhnPoker::print(). */
	percept_t m_agent_previous_card;

	/** The environment's (opponent's) action in the previous round. Used by
	 * KuhnPoker::print(). */
	action_t m_env_previous_action;

	/** The environments's (opponent's) card in the previous round. Used by
	 * KuhnPoker::print(). */
	percept_t m_env_previous_card;

	/** Action: agent bets an additional token. */
	static const action_t aBet;

	/** Action: agent passes (does not bet). */
	static const action_t aPass;

	/** Observation: the agent's card is a jack. */
	static const percept_t oJack;

	/** Observation: the agent's card is a queen. */
	static const percept_t oQueen;

	/** Observation: the agent's card is a king. */
	static const percept_t oKing;

	/** Observation: the environment (opponent) bety. */
	static const percept_t oBet;

	/** Observation: the environment (opponent) passed. */
	static const percept_t oPass;

	/** Reward: agent lost and bet. */
	static const percept_t rBetLoss;

	/** Reward: agent lost and passed. */
	static const percept_t rPassLoss;

	/** Reward: agent won and environment (opponent) passed. */
	static const percept_t rPassWin;

	/** Reward: agent won and environment (opponent) bet. */
	static const percept_t rBetWin;

	/** The probability that the environment (opponent) will bet on a king
	 * during its initial bet. */
	static const double cBetProbKing;

	/** The probability that the environment (opponent) will bet on a queen
	 * during its second bet. */
	static const double cBetProbQueen;

	/** The probability that the environment (opponent) will bet on a jack
	 * during its initial bet. */
	static const double cBetProbJack;
};

#endif // __KUHNPOKER_HPP__

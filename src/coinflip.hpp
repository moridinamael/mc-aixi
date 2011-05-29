#ifndef __COINFLIP_HPP__
#define __COINFLIP_HPP__

#include "environment.hpp"

/** A biased coin is flipped and the agent is tasked with predicting how it
 * will land. The agent receives a reward of CoinFlip::rWin for a correct
 * prediction and CoinFlip::rLoss for an incorrect prediction. The observation
 * specifies which side the coin landed on (CoinFlip::oTails or
 * CoinFlip::oHeads). The action corresponds to the agent's prediction for the
 * next coin flip (CoinFlip::aTails or CoinFlip::aHeads).
 *
 * Domain characteristics:
 * - environment: "coin-flip"
 * - maximum action: 1 (1 bit)
 * - maximum observation: 1 (1 bit)
 * - maximum reward: 1 (1 bit)
 *
 * Configuration options:
 * - coin-flip-p (optional): the probability the coin lands on heads
 *  (CoinFlip::oHeads). Must be a floating point number between 0.0 and 1.0
 *  inclusive. Default value is CoinFlip::cDefaultProbability. */
class CoinFlip : public Environment {
public:

	// set up the initial environment percept
	CoinFlip(options_t &options);

	virtual void performAction(const action_t action);

	virtual action_t maxAction() const { return 1; }
	virtual percept_t maxObservation() const { return 1; }
	virtual percept_t maxReward() const { return 1; }

	virtual std::string print() const;

private:
	/** Action: agent predicts the coin will land on tails. */
	static const action_t aTails;

	/** ACtion: agent predicts the coin will land on heads. */
	static const action_t aHeads;

	/** Observation: the coin lands on tails. */
	static const percept_t oTails;

	/** Observation: the coin lands on heads. */
	static const percept_t oHeads;

	/** Reward: agent incorrectly predicted the toss. */
	static const percept_t rLoss;

	/** Reward: agent correctly predicted the toss. */
	static const percept_t rWin;

	/** Probability of throwing heads (CoinFlip::oHeads). Default value is
	 * CoinFlip::cDefaultProbability. */
	double m_probability;

	/** Default value of CoinFlip::m_probability. */
	static const double cDefaultProbability;
};

#endif // __COINFLIP_HPP__

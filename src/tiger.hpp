#ifndef __TIGER_HPP__
#define __TIGER_HPP__
#include "environment.hpp"

/** The environment dynamics are as follows: a tiger and a pot of gold are
 * hidden behind one of two doors. Initially the agent starts facing both
 * doors. The agent has a choice of one of three actions: listen, open the
 * left door, or open the right door. If the agent opens the door hiding the
 * tiger, it suffers a -100 penalty. If it opens the door with the pot of gold,
 * it receives a reward of 10. If the agent performs the listen action, it
 * receives a penalty of -1 and an observation that correctly describes where
 * the tiger is with 0.85 probability.
 *
 * Domain characteristics:
 * - environment: "tiger"
 * - maximum action: 2 (2 bits)
 * - maximum observation: 2 (2 bits)
 * - maximum reward: 110 (7 bits)
 *
 * Configuration options:
 * - tiger-listen-accuracy (optional): the probability that listening for the
 *   tiger results in the correct observation of the tiger's whereabouts
 *   (Tiger::m_listen_accuracy). Must be a floating point number between 0.0
 *   and 1.0 inclusive. Default value is Tiger::cDefaultListenAccuracy.
 */
class Tiger : public Environment {
public:
	
	// set up the initial environment percept
	Tiger(options_t &options);
	
	// receives the agent's action and calculates the new environment percept
	virtual void performAction(const action_t action);
	
	virtual action_t maxAction() const { return 2; }
	virtual percept_t maxObservation() const { return 2; }
	virtual percept_t maxReward() const { return 110; }

	virtual std::string print() const;

private:
	/** Randomly place the tiger behind one door and the gold behind the other. */
	void placeTiger();
	
	/** The percept corresponding to the door which hides the gold. */
	percept_t m_gold;

	/** The percept corresponding to the door which hides the tiger. */
	percept_t m_tiger;

	/** The accuracy of the listen action. Default value is
	 * Tiger::cDefaultListenAccuracy. */
	double m_listen_accuracy;

	/** Default value for Tiger::m_listen_accuracy. */
	static const double cDefaultListenAccuracy;

	/** Action: listen for the tiger. */
	static const action_t aListen;

	/** Action: open left door. */
	static const action_t aLeft;

	/** Action: open right door. */
	static const action_t aRight;

	/** Observation: given when opening a door. */
	static const percept_t oNull;

	/** Observation: hear tiger at left door. */
	static const percept_t oLeft;

	/** Observation: hear tiger at right door. */
	static const percept_t oRight;

	/** Reward: eaten by tiger. */
	static const percept_t rEaten;

	/** Reward: for listening. */
	static const percept_t rListen;

	/** Reward: found gold. */
	static const percept_t rGold;
};

#endif // __TIGER_HPP__

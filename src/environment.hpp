#ifndef __ENVIRONMENT_HPP__
#define __ENVIRONMENT_HPP__

#include <string>

#include "main.hpp"
#include "util.hpp"

/** Base class for the various environments. Each individual environment should
 * inherit from this class and implement the appropriate methods. In particular,
 * the constructor should set up the environment as appropriate, including
 * setting the initial observation and reward, as well as setting appropriate
 * values for the configuration options:
 *  - "agent-actions"
 *  - "observation-bits"
 *  - "reward-bits"
 *
 * Following this, the agent and environment interact in a cyclic fashion. The
 * agent receives the observation and reward using Environment::getObservation()
 * and Environment::getReward() before supplying the environment with an action
 * via Environment::performAction(). Upon receiving an action, the environment
 * updates the observation and reward. At the beginning of each cycle, the value
 * of Environment::isFinished() is checked. If it is true then there is no
 * more interaction between the agent and environment and the program exits.
 * Otherwise the interaction continues indefinitely.
 */
class Environment {

public:

	// Constructor: set up the initial environment percept
	// Implement in inherited class

	virtual std::string print(void) const;

	/** Receives the agent's action and calculates the new environment percept. */
	virtual void performAction(action_t action) = 0;

	/** \return True if the environment cannot interact with the agent anymore. */
	virtual bool isFinished(void) const { return false; }

	/** \return The current observation. */
	percept_t getObservation(void) const { return m_observation; }

	/** \return The current reward. */
	percept_t getReward(void) const { return m_reward; }

	/** The maximum number of bits required to represent an action. */
	int actionBits() const { return bitsRequired(maxAction()); }

	/** The maximum number of bits required to represent an observation. */
	int observationBits() const { return bitsRequired(maxObservation()); }

	/** The maximum number of bits required to represent a reward. */
	int rewardBits() const { return bitsRequired(maxReward()); }

	/** The maximum number of bits required to represent a percept. */
	int perceptBits() const { return observationBits() + rewardBits(); }

	/** The maximum possible action. */
	virtual action_t maxAction() const = 0;

	/** The maximum possible observation. */
	virtual percept_t maxObservation() const = 0;

	/** The maximum possible reward. */
	virtual percept_t maxReward() const = 0;

	/** The minimum possible action. */
	virtual action_t minAction() const { return 0; };

	/** The minimum possible observation. */
	virtual percept_t minObservation() const { return 0; };

	/** The minimum possible reward. */
	virtual percept_t minReward() const { return 0; };

	/** Checks if an action is valid. */
	virtual bool isValidAction(const action_t action) const;

	/** Checks if an observation is valid (i.e. possible to observe). */
	virtual bool isValidObservation(const percept_t observation) const;

	/** Checks if a reward is valid (i.e. possible to observe). */
	virtual bool isValidReward(const percept_t reward) const;


protected: // visible to inherited classes

	/** The last action performed by the agent. */
	action_t m_action;

	/** The current observation. */
	percept_t m_observation;

	/** The current reward. */
	percept_t m_reward;

};


#endif // __ENVIRONMENT_HPP__

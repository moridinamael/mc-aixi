#ifndef __AGENT_HPP__
#define __AGENT_HPP__

#include <iostream>
//#include <queue>
#include "environment.hpp"
#include "main.hpp"

class ContextTree;

class SearchNode;

class ModelUndo;

enum update_t {action_update, percept_update};

/** The ::Agent class represents a MC-AIXI-CTW agent.  It includes much of the
 * high-level logic for choosing suitable actions. In particular, the agent
 * maintains an internal model of the environment using a context tree
 * Agent::m_ct. It uses this internal model to to predict the probability of
 * future outcomes:
 *  - Agent::getPredictedActionProb()
 *  - Agent::perceptProbability()
 *
 * as well as to generate actions and percepts according to the model
 * distribution:
 *  - Agent::genAction()
 *  - Agent::genPercept()
 *  - Agent::genPerceptAndUpdate()
 *  - Agent::genRandomAction()
 *
 * Actions are chosen via the UCT algorithm, which is orchestrated by a
 * high-level search function and a playout policy:
 *  - Agent::search()
 *  - Agent::playout()
 *  - Agent::m_horizon
 *  - Agent::m_mc_simulations
 *  - Agent::m_search_tree
 *
 * Several functions decode/encode actions and percepts between the
 * corresponding types (i.e. ::action_t, ::percept_t) and generic
 * representation by symbol lists:
 *  - Agent::decodeAction()
 *  - Agent::decodeObservation()
 *  - Agent::decodePercept()
 *  - Agent::decodeReward()
 *  - Agent::encodeAction()
 *  - Agent::encodePercept()
 *
 * There are various attributes which describe the agent and it's
 * interaction with the environment so far:
 *  - Agent::age()
 *  - Agent::averageReward()
 *  - Agent::historySize()
 *  - Agent::horizon()
 *  - Agent::lastUpdate()
 *  - Agent::maxAction()
 *  - Agent::maxBitsNeeded()
 *  - Agent::maxReward()
 *  - Agent::totalReward()
 */
class Agent {

public:

	/** The maximum possible reward the agent can receive in a single cycle. */
	double maxReward() const { return m_env.maxReward(); }

	/** The "maximum" action the agent can execute. */
	int maxAction() const { return m_env.maxAction(); }

	/** Construct a learning agent from the configuration arguments and
	 * environmet.
	 * \param options The configuration options.
	 * \param env The environment the agent will interact with. */
	Agent(options_t & options, Environment const& env);

	/** Destry the agent and the corresponding context tree. */
	~Agent();

	/** Current age of the agent in cycles. */
	age_t age() const;

	/** The total accumulated reward across an agents lifespan. */
	reward_t totalReward() const;

	/** The average reward received by the agent at each time step. */
	reward_t averageReward() const;

	/** The length of the stored history for an agent. */
	int historySize() const;

	/** The length of the search horizon used by the agent. */
	int horizon() const;

	/** True if the last update was a percept, false if it was an action. */
	update_t lastUpdate(void) const { return m_last_update; };

	/** The maximum number of bits to encode either an action or percept. */
	int maxBitsNeeded() const;

	int modelSize() const;

	/** Generate an action uniformly at random.
	 * \return The generated action. */
	action_t genRandomAction() const;

	/** Generate an action distributed according to the agent's history
	 * statistics by doing rejection sampling from the context tree.
	 * \return The generated action. */
	action_t genAction() const;

	/** Generate a percept distributed according to the agent's history
	 * statistics by sampling from the context tree.
	 * \param observation Receives the observation part of the generated
	 *     percept.
	 * \param reward Receives the reward part of the generated percept. */
	void genPercept(percept_t &observation, percept_t &reward);

	/** Generate a percept distributed according to the agent's history
	 * statistics, and update the context tree with it.
	 * \param observation Receives the observation part of the generated
	 * 	   percept.
	 * \param reward Receives the reward part of the generated percept. */
	void genPerceptAndUpdate(percept_t &observation, percept_t &reward);

	/** Update the agent's model of the world with a percept from the
	 * environment
	 * \param observation The observation that was received.
	 * \param reward The reward that was received. */
	void modelUpdate(percept_t observation, percept_t reward);

	/** Update the agent's model of the world after performing an action.
	 * \param action The action that the agent performed. */
	void modelUpdate(action_t action);

	/** Revert the agent's model of the world to that of a previous time cycle. */
	void modelRevert(const ModelUndo &mu);

	/** Resets the agent and clears the context tree. */
	void reset(void);

	/** Probability of selecting an action according to the
	 * agent's internal model of it's own behaviour.
	 * \param action The action we wish to find the likelihood of.
	 * \return The probability of the agent selecting action. */
	double getPredictedActionProb(action_t action);

	/** Probability of receiving a particular percept (observation and reward)
	 * according to the agent's environment model.
	 * \param observation The observation part of the percept we wish to find
	 * the likelihood of.
	 * \param reward The reward part of the percept we wish to find the
	 * likelihood of.
	 * \returns The probability of observing the (observation, reward) pair. */
	double perceptProbability(percept_t observation, percept_t reward) const;

	/** Determine the best action for the agent using Monte-Carlo Tree Search
	 * (predictive UCT).
	 * \return The best action as determined by the sampling. */
	action_t search(void);

	/** Simulate agent/enviroment interaction for a specified amount of steps
	 * where agent actions are chosen uniformly at random and percepts are generated
	 * from the agents environment model.
	 * \param agent The agent doing the sampling.
	 * \param playout_len The number of complete action/percept steps to simulate.
	 * \return The total reward from the simulation. */
	reward_t playout(int horizon);

private:


	/** Encode an action as a list of symbols.
	 * \param symlist The symbol list to encode the action to.
	 * \param action The action to encode. */
	void encodeAction(symbol_list_t &symlist, action_t action) const;

	/** Encode a percept as a list of symbols.
	 * \param symlist The symbol list to encode the percept to.
	 * \param observation The observation part of the percept to encode.
	 * \param reward The reward part of the percept to encode. */
	void encodePercept(symbol_list_t &symlist, percept_t observation, percept_t reward) const;

	/** Decode an action from the beginning of a list of symbols.
	 * \param symlist The symbol list to decode the action from.
	 * \return The decoded action. */
	action_t decodeAction(const symbol_list_t &symlist) const;

	/** Decode a reward from the beginning of a list of symbols.
	 * \param symlist The symbol list to decode the reward from.
	 * \return The decoded reward. */
	percept_t decodeReward(const symbol_list_t &symlist) const;

	/** Decode an observation from the beginning of a list of symbols.
	 * \param symlist The symbol list to decode the observation from.
	 * \return The decoded observation. */
	percept_t decodeObservation(const symbol_list_t &symlist) const;

	/** Decode a percept (observation and reward) from the beginning of a list
	 * of symbols.
	 * \param symlist The symbol list to decode the observation from.
	 * \param observation Receives the decoded observation.
	 * \param reward Receives the decoded reward. */
	void decodePercept(const symbol_list_t &symlist, percept_t &observation, percept_t &reward);


	/** Stores the configuration options. */
	options_t &m_options;

	/** A reference to the environment the agent interacts with. */
	Environment const& m_env;

	/** Context tree representing the agent's model of the environment. */
	ContextTree *m_ct;

	/** The number of interaction cycles the agent has been alive. */
	age_t m_time_cycle;

	/** The total reward received by the agent. */
	reward_t m_total_reward;

	/** The type of the last update (action or percept). */
	update_t m_last_update;
	
	/** The length of the agent's planning horizon. */
	int m_horizon;

	/** The number of simulations to conduct when choosing new actions via the
	 * UCT algorithm. */
	int m_mc_simulations;

	/** The root node of the UCT search tree. */
	SearchNode *m_search_tree;

	/** The number of cycles during which the agent learns. */
	int m_learning_period;
};


/** The ::ModelUndo class is used to store the information required to restore
 * an agent to a copy of itself from a previous time cycle. In particular, it
 * is sufficient to back up the following attributes:
 *  - Agent::age()
 *  - Agent::reward()
 *  - Agent::historySize()
 *  - Agent::lastUpdatePercept() */
class ModelUndo {
public:
	/** Extracts the information required to restore an agent to its current
	 * state.
	 * \param agent The agent whose state we wish to "save". */
	ModelUndo(const Agent &agent);

	/** The age of the agent (Agent::age()) at the time it was saved. */
    age_t age(void) const { return m_age; }

    /** The total reward of the agent (Agent::reward()) at the time it was
     * saved. */
    reward_t reward(void) const { return m_reward; }

    /** The size of the agents history size (Agent::historySize()) at the time
     * it was saved. */
    size_t historySize(void) const { return m_history_size; }

    /** The status of the agents last update (Agent::lastUpdatePercept()) at
     * the time it was saved. */
    update_t lastUpdate(void) const { return m_last_update; }

private:
    age_t m_age;
    reward_t m_reward;
    size_t m_history_size;
    update_t m_last_update;
};


#endif // __AGENT_HPP__

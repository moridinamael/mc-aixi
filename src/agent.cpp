#include <cassert>
#include <iostream>

#include "agent.hpp"
#include "predict.hpp"
#include "search.hpp"
#include "util.hpp"

// construct a learning agent from the command line arguments
Agent::Agent(options_t &options, Environment const& env) :
	m_options(options), m_env(env)
{
	m_time_cycle = 1;

	getRequiredOption(options, "agent-horizon", m_horizon);
	getRequiredOption(options, "mc-simulations", m_mc_simulations);
	getOption(options, "learning-period", 0, m_learning_period);

	// Create context tree
	int ct_depth = getRequiredOption<int>(options, "ct-depth");
	m_ct = new ContextTree(ct_depth);

	reset();
}


// destroy the agent and the corresponding context tree
Agent::~Agent(void) {
	if (m_ct)
		delete m_ct;
}


// current age of the agent in cycles
age_t Agent::age(void) const {
	return m_time_cycle;
}


// the total accumulated reward across an agents lifespan
reward_t Agent::totalReward(void) const {
	return m_total_reward;
}


// the average reward received by the agent at each time step
reward_t Agent::averageReward(void) const {
	return age() > 0 ? totalReward() / reward_t(age()) : 0.0;
}


// the length of the stored history for an agent
int Agent::historySize(void) const {
	return m_ct->historySize();
}


// length of the search horizon used by the agent
int Agent::horizon(void) const {
	return m_horizon;
}


// maximum number of bits needed to represent action or percept
int Agent::maxBitsNeeded() const {
	return std::max(m_env.actionBits(), m_env.perceptBits());
}

int Agent::modelSize() const {
	return m_ct->size();
}


// generate an action uniformly at random
action_t Agent::genRandomAction(void) const {
	return randRange(m_env.maxAction() + 1);
}


// generate an action distributed according to our history statistics
action_t Agent::genAction(void) const {
	assert(m_last_update == percept_update);

	// sample from context tree
	symbol_list_t action_syms;
	m_ct->genRandomSymbols(action_syms, m_env.actionBits());

	// decode sample
	return decodeAction(action_syms);
}


// generate a percept distributed according to our history statistics.
void Agent::genPercept(percept_t &o, percept_t &r) {
	// sample from context tree
	symbol_list_t percept_syms;
	m_ct->genRandomSymbols(percept_syms, m_env.perceptBits());

	decodePercept(percept_syms, o, r);
}


// generate a percept distributed to our history statistics, and
// update our mixture environment model with it
void Agent::genPerceptAndUpdate(percept_t &o, percept_t &r) {
	// sample from context tree
	symbol_list_t percept_syms;
	m_ct->genRandomSymbolsAndUpdate(percept_syms, m_env.perceptBits());
	decodePercept(percept_syms, o, r);

	// Update other properties
	m_total_reward += r;
	m_last_update = percept_update;
}


// Update the agent's internal model of the world after receiving a percept
void Agent::modelUpdate(percept_t observation, percept_t reward) {
	assert(m_last_update == action_update);

	// Update internal model
	symbol_list_t percept_syms;
	encodePercept(percept_syms, observation, reward);
	if(m_learning_period > 0 && m_time_cycle > m_learning_period)
		m_ct->updateHistory(percept_syms); // Update but don't learn
	else
		m_ct->update(percept_syms); // Update and learn

	// Update other properties
	m_total_reward += reward;
	m_last_update = percept_update;
}

// Update the agent's internal model of the world after performing an action
void Agent::modelUpdate(const action_t action) {
	assert(m_env.isValidAction(action));
	assert(m_last_update == percept_update);

	// Update internal model
	symbol_list_t action_syms;
	encodeAction(action_syms, action);
	m_ct->updateHistory(action_syms);

	m_time_cycle++;
	m_last_update = action_update;
}


// revert the agent's internal model of the world
// to that of a previous time cycle
void Agent::modelRevert(const ModelUndo &mu) {

	// Revert excess actions and percepts
	while (historySize() > mu.historySize()) {

		if(m_last_update == percept_update) { // Undo percept
			m_ct->revert(m_env.perceptBits());
			m_last_update = action_update;
		} else {                              // Undo action
			m_ct->revertHistory(m_env.actionBits());
			m_last_update = percept_update;
		}
	}

	// revert agent parameters
	m_time_cycle = mu.age();
	m_total_reward = mu.reward();
	m_last_update = mu.lastUpdate();
}


void Agent::reset(void) {
	m_ct->clear();
	m_time_cycle = 0;
	m_total_reward = 0.0;
	m_last_update = action_update;
}


// probability of selecting an action according to the
// agent's internal model of it's own behaviour
double Agent::getPredictedActionProb(const action_t action) {
	// sanity checks
	assert(m_env.isValidAction(action));
	assert(m_last_update == percept_update);

	// encode action
	symbol_list_t action_syms;
	encodeAction(action_syms, action);

	// predict using context tree
	return m_ct->predict(action_syms);
}


// get the agent's probability of receiving a particular percept
double Agent::perceptProbability(const percept_t observation, const percept_t reward) const {
	assert(m_last_update == action_update);

	// encode percept
	symbol_list_t percept;
	encodePercept(percept, observation, reward);

	// predict using context tree
	return m_ct->predict(percept);
}


// Use rhoUCT to search for next action.
action_t Agent::search(void) {
	// Save the agent's current state
	ModelUndo undo = ModelUndo(*this);

	// Create a new search tree
	m_search_tree = new SearchNode(decision);


	// Main sampling loop
	for (int t = 0; t < m_mc_simulations; t++) {
		m_search_tree->sample(*this, m_horizon);
		modelRevert(undo);
	}

	// Determine best action using tree constructed during sampling
	// by choosing the action branch from this tree that provides the best expected reward.
	action_t best_action = genRandomAction();
	double best_mean = -1;

	for (action_t a = 0; a <= maxAction(); a++) {
		if (!m_search_tree->child(a))
			continue;

		double mean = m_search_tree->child(a)->expectation() + rand01() * 0.0001;
		if (mean > best_mean) {
			best_mean = mean;
			best_action = a;
		}
	}

	delete m_search_tree;

	return best_action;
}


// Agent's playout policy. Generate percepts from context tree and choose
// actions uniformly at random.
reward_t Agent::playout(int horizon) {

	reward_t reward = 0.0;
	while (horizon-- > 0) {
		// Execute an action chosen uniformly at random.
		action_t a = genRandomAction();
		modelUpdate(a);

		// Sample a percept.
		percept_t o, r;
		genPerceptAndUpdate(o, r);
		reward += r;
	}

	return reward;
}


// Encodes an action as a list of symbols
void Agent::encodeAction(symbol_list_t &symbols, action_t action) const {
	symbols.clear();
	encode(symbols, action, m_env.actionBits());
}


// Encodes a percept (observation, reward) as a list of symbols
void Agent::encodePercept(symbol_list_t &symlist, percept_t observation,
		percept_t reward) const {
	symlist.clear();

	encode(symlist, reward, m_env.rewardBits());
	encode(symlist, observation, m_env.observationBits());
}


// Decodes the action from a list of symbols
// When m_actions doesn't fill bits, just wrap around
action_t Agent::decodeAction(const symbol_list_t &symlist) const {
	return decode(symlist, m_env.actionBits()) % (m_env.maxAction() + 1);
}


// Decodes the observation from a list of symbols
percept_t Agent::decodeObservation(const symbol_list_t &symlist) const {
	return decode(symlist, m_env.observationBits());
}


// Decodes the reward from a list of symbols
percept_t Agent::decodeReward(const symbol_list_t &symlist) const {
	return decode(symlist, m_env.rewardBits());
}


// Decode the percept (observation, reward) from a list of symbols
void Agent::decodePercept(const symbol_list_t &symlist, percept_t &observation,
		                  percept_t &reward) {

	symbol_list_t reward_syms = symbol_list_t(m_env.rewardBits());
	copy(symlist.begin(), symlist.begin() + m_env.rewardBits(),
	     reward_syms.begin());

	symbol_list_t observation_syms = symbol_list_t(m_env.observationBits());
	copy(symlist.begin() + m_env.rewardBits(), symlist.end(),
	     observation_syms.begin());

	reward = decodeReward(reward_syms);
	observation = decodeObservation(observation_syms);
}


// used to revert an agent to a previous state
ModelUndo::ModelUndo(const Agent &agent) {

	m_age = agent.age();
	m_reward = agent.totalReward();
	m_history_size = agent.historySize();
	m_last_update = agent.lastUpdate();
}

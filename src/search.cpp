#include <cassert>
#include <cmath>
#include <limits>
#include "agent.hpp"
#include "search.hpp"
#include "util.hpp"

/** Exploration constant for UCB action policy. */
static const double exploration_constant = 2.0;

SearchNode::SearchNode(const nodetype_t nodetype) {
	m_mean = 0;
	m_visits = 0;
	m_type = nodetype;
}

SearchNode::~SearchNode(void) {
	child_map_t::iterator child_iter = m_child.begin();
	for( ; child_iter != m_child.end(); child_iter++) {
		delete child_iter->second;
	}
}

// Select an action according to UCB policy
action_t SearchNode::selectAction(Agent const& agent) {
	const double explore_bias = agent.horizon() * agent.maxReward();
	const double unexplored_bias = 1000000000.0;
	const double log_visits = std::log((double) visits());

	// Compute the best action according to the UCB formula.
	action_t best_action;
	double best_priority = -std::numeric_limits<double>::infinity();
	for (action_t a = 0; a <= agent.maxAction(); a++) {
		SearchNode *n = child(a);

		// Use UCB formula to determine priority of node
		double priority = 0.0;
		if (n == NULL || n->visits() == 0) { // Previously unexplored node
			priority = unexplored_bias;
		} else {                             // Previously explored node
			double nvisits = double(n->visits());
			priority = n->expectation() + explore_bias
				* std::sqrt(exploration_constant * log_visits / nvisits);
		}

		// Update best action if necessary, breaking ties randomly.
		if (priority > best_priority + rand01() * 0.001) {
			best_action = a;
			best_priority = priority;
		}
	}

	return best_action;
}


reward_t SearchNode::sample(Agent &agent, const int horizon) {
	reward_t reward = 0.0;

	// If we have reached the agents horizon or the maximum search depth then
	// return the reward. Otherwise continuing sampling or use a playout policy
	// as appropriate.
	if (horizon == 0) {
		return 0.0; // Reached agent horizon
	}
	else if (m_type == chance) {
		// We are at a chance node, generate a percept at random using the
		// agents environment model and continue sampling.
		percept_t o, r;
		agent.genPerceptAndUpdate(o, r);

		if (!child(o))
			m_child[o] = new SearchNode(decision);
		reward = r + m_child[o]->sample(agent, horizon - 1);
	}
	else if (visits() == 0) {
		// We are at a decision node. Either the node is previously unvisited or
		// we have exceeded the maximum tree depth. Either way, use the playout
		// policy to estimate the future reward.
		reward = agent.playout(horizon);
	}
	else {
		// We are at a decision node, choose an action according to the UCB
		// policy and continue sampling.
		action_t a = selectAction(agent);
		agent.modelUpdate(a);

		if (child(a) == NULL)
			m_child[a] = new SearchNode(chance);
		reward = m_child[a]->sample(agent, horizon);
	}

	// Update the expected reward and number of visits to the current node.
	double v = double(visits());
	m_mean = (reward + v * expectation()) / (v + 1.0);
	m_visits++;
	return reward;
}


SearchNode *SearchNode::child(const interaction_t child_index) const {
	child_map_t::const_iterator c = m_child.find(child_index);
	return c == m_child.end() ? NULL : c->second;
}



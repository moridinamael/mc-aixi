#ifndef __SEARCH_HPP__
#define __SEARCH_HPP__
#include "main.hpp"

class Agent;

class SearchNode;

/** Type for storing the number of visits to a node. */
typedef long long visits_t;

/** Used to specify the type of SearchNode. Chance nodes represent a set
 * of possible observation (one child per observation) while decision nodes
 * represent sets of possible actions (one child per action). Decision and
 * chance nodes alternate. */
enum nodetype_t { chance, decision };

/** Mapping used to store the children of a SearchNode. Indexed by actions or
 * percepts as appropriate. */
typedef std::map<interaction_t, SearchNode*> child_map_t;



/** Represents a node in the Monte Carlo search tree. The nodes in the search
 * tree represent simulated actions and percepts between an agent following a
 * UCB policy and a generative model of the environment represented by a context
 * tree. The purpose of the tree is to determine the expected reward of the
 * available actions through sampling. Sampling proceeds several time steps
 * into the future according to the size of the agent's horizon
 * (Agent::horizon())
 *
 * The nodes are one of two types (::nodetype_t), decision nodes are those
 * whose children represent actions from the agent and chance nodes are those
 * whose children represent percepts from the environment. Each SearchNode
 * maintains several bits of information
 *  - The current value of the sampled expected reward (SearchNode::m_mean,
 *    SearchNode::expectation()).
 *  - The number of times the node has been visited during the sampling
 *    (SearchNode::m_visits, SearchNode::visits()).
 *  - The type of the node (SearchNode::m_type).
 *  - The children of the node (SearchNode::m_child, SearchNode::child()). The
 *    children are stored in a map of type ::child_map_t and are indexed by
 *    actions (decision node) or percepts (chance node).
 *
 * The SearchNode::sample() function is used to sample from the current node and
 * the SearchNode::selectAction() is used to select an action according to the
 * UCB policy. */
class SearchNode {

public:

	/** Create and initialise a new search node of a specific type. */
	SearchNode(const nodetype_t nodetype);

	/** Destroy all child nodes. */
	~SearchNode(void);

	/** Determine which action to sample according to the UCB policy.
	 * \param agent The agent which is doing the sampling.
	 * \return The selected action. */
	action_t selectAction(Agent const& agent);

	/** \return The sampled expected reward from this node. */
	reward_t expectation(void) const { return m_mean; }

	/** Perform a single sample from this node.
	 * \param agent The agent which is doing the sampling.
	 * \param horizon How many cycles into the future to sample.
	 * \return The accumulated reward from this sample. */
	reward_t sample(Agent &agent, const int horizon );

	/** \return The number of times this node has been visited. */
	visits_t visits(void) const { return m_visits; }

	/** Attempts to access the child node with a certain index.
	 * \param child_index The index of the child node. This corresponds to an
	 * action if the node is a decision node or a percept if the node is a
	 * chance node.
	 * \return A pointer to the child node if it exists, otherwise return NULL. */
	SearchNode *child(const interaction_t child_index) const;

private:
	/** The children of this node. Each corresponds to an action if this is a
	 * decision node or to a percept if this is a chance node. */
	child_map_t m_child;

	/** The type of this node indicates whether it's children represent actions
	 * (decision node) or percepts (chance node). */
	nodetype_t m_type;

	/** The sampled expected reward of this node. */
	double m_mean;

	/** The number of times this node has been visited. */
	visits_t m_visits;
};


#endif // __SEARCH_HPP__

#ifndef __PREDICT_HPP__
#define __PREDICT_HPP__
#include <vector>
#include "main.hpp"

/** Stores symbol occurrence counts. */
typedef int count_t;

/** Holds context weights. */
typedef double weight_t;

/** The ::CTNode class represents a node in an action-conditional context tree. The
 * purpose of each node is to calculate the weighted probability of observing
 * a particular bit sequence. In particular, denote by \f$ n \f$ the
 * current node, by \f$ n0 \f$ and \f$ n1 \f$ the child nodes, by \f$ h_n \f$
 * the subsequence of the history relevant to node \f$ n \f$, and by \f$ a \f$
 * and \f$ b \f$ the number of zeros and ones in \f$ h_n \f$. Then the weighted
 * block probability of observing \f$ h_n \f$ at node \f$ n \f$ is given by
 * \f[
 *     P_w^n(h_n) :=
 *     \begin{cases}
 *         \Pr_\text{kt}(h_n) & \text{if } n \text{ is a leaf node} \\
 *         \frac{1}{2} \Pr_\text{kt}(h_n) +
 *         \frac{1}{2} P_w^{n0}(h_{n0}) P_w^{n1}(h_{n1})
 *         & \text{otherwise}
 *     \end{cases}
 * \f]
 * where \f$ \Pr_\text{kt}(h_n) = \Pr_\text{kt}(a, b) \f$ is the
 * Krichevsky-Trofimov (KT) estimator defined by the relations
 * \f[
 *     \Pr_\text{kt}(a + 1, b) = \frac{a + 1/2}{a + b + 1} \Pr_\text{kt}(a, b)
 * \f]
 * \f[
 *     \Pr_\text{kt}(a, b + 1) = \frac{b + 1/2}{a + b + 1} \Pr_\text{kt}(a, b)
 * \f]
 * and the base case \f$ \Pr_\text{kt}(0, 0) := 1 \f$. In both relations, the
 * fraction is referred to as the update multiplier and corresponds to the
 * probability of observing a zero (first relation) or a one (second relation)
 * given we have seen \f$ a \f$ zeros and \f$ b \f$ ones.
 *
 * Due to numerical issues, the implementation uses logarithmic probabilities
 * \f$ \ln P_w^n(h_n) \f$ and \f$ \ln \Pr_\text{kt}(h_n) \f$ rather than normal
 * probabilities. These probabilities are recalculated during updates
 * (CTNode::update()) and reversions (CTNode::revert()) to the context tree that
 * involve the node.
 *  - The KT estimate is stored using CTNode::m_log_kt and accessed
 *    through CTNode::logKT(). It is updated from the previous
 *    estimate by multiplying with the update multiplier as calculated by
 *    CTNode::logKTMultiplier().
 *  - The weighted probability is stored using CTNode::m_log_probability and
 *    accessed through CTNode::logProbability(). It is recalculated by
 *    CTNode::updateLogProbability().
 *
 * In order to calculate these probabilities, ::CTNode also stores:
 *  - Links to child nodes: CTNode::child(), CTNode::m_child.
 *  - The number of zeros and ones in the history subsequence relevant to the
 *    node: CTNode::m_count.
 *
 *
 * The ::CTNode class is tightly coupled with the ::ContextTree class. Briefly,
 * the ::ContextTree class
 *  - Creates and deletes nodes.
 *  - Tells the appropriate nodes to update/revert their probability estimates.
 *  - Samples actions and percepts from the probability distribution specified
 *    by the nodes. */
class CTNode {
	/** The ::ContextTree class is made a friend so it can access the private
	 * members of ::CTNode. There are several reasons for this:
	 *  - The log weighted block probability and log KT estimated block
	 *    probability are calculated by the ContextTree::update() method and the
	 *    CTNode::logProbWeighted() and CTNode::logProbEstimated() methods
	 *    simply return these calculated values.
	 *  - This arrangement allows the ::ContextTree class to create/delete
	 *    nodes from the context tree. */
	friend class ContextTree;

public:

	/** Retrieves the cached KT estimate of the log probability of the history
	 * subsequence relevant to this node. The value is computed only when the
	 * node is changed (by CTNode::update() or CTNode::revert()) and is cached
	 * in the variable CTNode:m_log_kt.
	 *
	 * \return The log KT estimate \f$ \ln \Pr_{kt} (h_{T, n}) = \ln
	 * \Pr_{kt}(0^a 1^b) = \ln \Pr_{kt}(a, b) \f$ where \f$ a \f$ and \f$ b \f$
	 * denote the number of zeros and ones in the history subsequence
	 * \f$ h_{T, n} \f$ relevant to this node \f$ n \f$. */
	weight_t logKT(void) const { return m_log_kt; }


	/** Retrieves the cached weighted log probability of the history subsequence
	 * relevant to this node. The value is computed only when the node is
	 * changed (by CTNode::update() or CTNode::revert()) and is cached in the
	 * variable CTNode::m_log_probability.
	 *
	 * \return The log weighted probability \f$ \ln P_w^n \f$. */
	weight_t logProbability(void) const { return m_log_probability; }


	/** The child node corresponding to a particular symbol. */
	const CTNode *child(const symbol_t sym) const {	return m_child[sym]; }


	/** Checks if this is a leaf node.
	 * \return True if the node is a leaf node, false otherwise. */
	bool isLeafNode(void) const {
		return (child(false) == NULL) && (child(true) == NULL);
	}


	/** The number of nodes in the tree rooted at this node. */
	int size(void) const;


	/** The number of times this context has been visited. This is equivalent to
	 * the sum of the values in CTNode::m_count as well as to the sum of the
	 * visits of the (immediate) child nodes. */
	int visits(void) const { return m_count[false] + m_count[true]; }


private:
	/** Initialise the node. */
	CTNode(void);


	/** Destroy the node and all children. */
	~CTNode(void);


	/** Compute the logarithm of the KT-estimator update multiplier. The
	 * log KT estimate of the conditional probability of observing a zero given
	 * we have observed \f$ a \f$ zeros and \f$ b \f$ ones at the current node is
	 * \f[
	 *     \ln \Pr_\text{kt}(0 \,|\, 0^a1^b) = \ln \frac{a + 1/2}{a + b + 1}.
	 * \f]
	 * Similarly, the estimate of the conditional probability of observing a
	 * one is
	 * \f[
	 *     \ln \Pr_\text{kt}(1 \,|\, 0^a1^b) = \ln \frac{b + 1/2}{a + b + 1}.
	 * \f]
	 *
	 * \param symbol The symbol for which to calculate the log KT estimate of
	 * conditional probability. False corresponds to calculating \f$ \ln
	 * \Pr_\text{kt}(0 \,|\, 0^a1^b) \f$ and true corresponds to calculating
	 * \f$ \ln \Pr_\text{kt}(1 \,|\, 0^a1^b) \f$.
	 * \return The log KT estimate of the conditional probability (update
	 * multiplier). */
	weight_t logKTMultiplier(const symbol_t symbol) const;


	/** Calculates the logarithm of the weighted block probability
	 * \f[
	 *     \ln P^n_w :=
	 *     \begin{cases}
	 *         \ln \Pr_\text{KT}(h_n) & \text{if } n \text{ is a leaf node} \\
	 *         \ln \left( \frac{1}{2} \Pr_\text{KT}(h_n)
	 *                    + \frac{1}{2} P^{n^0}_w \times P^{n^1}_w \right)
	 *         & \text{otherwise}
	 *     \end{cases}
	 * \f]
	 * and stores the value in CTNode::m_log_probability.
	 *
	 * Because of numerical issues, the implementation works directly with the
	 * log probabilities \f$ \ln \Pr_\text{KT}(h_n) \f$, \f$ \ln P_w^{n^0} \f$,
	 * and \f$ \ln P_w^{n^1} \f$ rather than the normal probabilities. To
	 * compute the second case of the weighted probability, we use the identity
	 * \f[
	 *     \ln (a + b) = \ln a + \ln( 1 + \exp( \ln b - \ln a ) ) \qquad a,b > 0
	 * \f]
	 * to rearrange so that logarithms act directly on the probabilities:
	 * \f[
	 *     \ln \left( \frac{1}{2} \Pr_\text{KT}(h_n) +
	 *                \frac{1}{2} P_w^{n0} P_w^{n1} \right) =
	 *     \begin{cases}
	 *         \ln (1/2) + \ln \Pr_\text{KT}(h_n)
	 *         + \ln \left( 1 + \exp \left( \ln P_w^{n0} + \ln P_w^{n1}
	 *                                      - \ln \Pr_\text{KT}(h_n) \right)
	 *               \right) \\
	 *         \ln (1/2) + \ln P_w^{n0} + \ln P_w^{n1}
	 *         + \ln \left( 1 + \exp \left( \ln \Pr_\text{KT}(h_n)
	 *                                      - \ln P_w^{n0} + \ln P_w^{n1}
	 *                               \right) \right)
	 * \end{cases}.
	 * \f]
	 * In order to avoid overflow problems, we choose the formulation for which
	 * the argument of the exponent \f$ \exp(\ln b - \ln a) \f$ is as small as
	 * possible. */
	void updateLogProbability(void);


	/** Update the node after having observed a new symbol. This involves
	 * updating the symbol counts and recalculating the cached probabilities.
	 * \param The symbol that was observed. */
	void update(const symbol_t symbol);


	/** Return the node to its state immediately prior to the last update. This
	 * involves updating the symbol counts, recalculating the cached
	 * probabilities, and deleting unnecessary child nodes.
	 * \param symbol The symbol used in the previous update. */
	void revert(const symbol_t symbol);


	/** The cached KT estimate of the block log probability for this node. */
	weight_t m_log_kt;


	/** The cached weighted log probability for this node. */
	weight_t m_log_probability;


	/** The number of zeros (CTNode::m_count[0]) and ones (CTNode::m_count[1])
	 * in the history subsequence relevant to this node. */
	int m_count[2];


	/** The children of this node. */
	CTNode *m_child[2];
};



/** The high-level interface to an action-conditional context tree. Most of the
 * mathematical details are implemented in the CTNode class, which is used to
 * represent the nodes of the tree. ContextTree stores a reference to the root
 * node of the tree (ContextTree::m_root), the history of updates to the tree
 * (ContextTree::m_history), and the maximum depth of the tree
 * (ContextTree::m_depth). It is primarily concerned with calling the
 * appropriate functions in the appropriate nodes in order to deliver certain
 * functionality:
 * - Updating the context tree and reverting previously made updates.
 *   - ContextTree::update(symbol_t) and
 *     ContextTree::update(const symbol_list_t&) update the tree and the history
 *     after the agent has observed new percepts.
 *   - ContextTree::updateHistory(symbol_t) and
 *     ContextTree::updateHistory(const symbol_list_t&) update just the history
 *     after the agent has executed an action.
 *   - ContextTree::revert() undoes the last update to the tree.
 *   - ContextTree::revertHistory() deletes the recent history.
 * - Predicting the probability of future outcomes (ContextTree::predict()).
 * - Sampling sequences of symbols from the context tree statistics.
 *   - ContextTree::genRandomSymbolAndUpdate() samples a sequence from the
 *     context tree, updating the tree with each bit as it is sampled.
 *   - ContextTree::genRandomSymbols() samples a sequence of a specified length,
 *     updating the tree with each bit as it is sampled, then reverting all the
 *     updates so that the tree is in the same state as it was before the
 *     sampling.
 */
class ContextTree {
public:

	/** Create a context tree of specified maximum depth. Only allocates memory
	 * for the root node, other nodes are created lazily as needed.
	 *
	 * \param depth The maximum depth of the context tree. */
	ContextTree(const int depth);


	/** Destroy the context tree and all the nodes referenced by the tree. */
	~ContextTree(void);


	/** Clears the entire context tree including all nodes and history. */
	void clear(void);


	/** Update the context tree with a new binary symbol. Recalculate the
	 * log weighted probabilities and log KT estimates for each affected node.
	 *
	 * \param symbol The symbol with which to update the tree. */
	void update(const symbol_t symbol);


	/** Update the context tree with a list of symbols. Equivalent to calling
	 * ContextTree::update() once for each symbol.
	 *
	 * \param symbols The symbols with which to update the tree. The context
	 *  tree is updated with symbols in the order they appear in the list. */
	void update(symbol_list_t const& symbols);


	/** Append a symbol to the history without updating the context tree.
	 *
	 * \param symbol The symbol to add to the history. */
	void updateHistory(const symbol_t symbol);


	/** Append symbols to the history without updating the context tree.
	 *
	 * \param symbols The list of symbols to add to the history. */
	void updateHistory(symbol_list_t const& symbols);


	/** Restores the context tree to as it was immediately prior to the previous
	 * update (CTNode::update()). */
	void revert(void);


	/** Restores the context tree to its state prior to a specified number of
	 * updates
	 * \param num_symbols The number of updates (symbols) to revert. */
	void revert(const int num_symbols);

	/** Shrinks the history down to a former size without changing the context
	 * tree. */
	void revertHistory(const int num_symbols);


	/** The estimated probability of observing a particular symbol. Given a
	 * history sequence \f$ h \f$ and a symbol \f$ y \f$, the estimated
	 * probability is given by \f[ \rho(y | h) = \frac{\rho(hy)}{\rho(h)} \f]
	 * where \f$ \rho(h) = P_w^\epsilon(h) \f$ is the weighted probability
	 * estimate of observing \f$ h \f$ evaluated at the root node
	 * \f$ \epsilon \f$ of the context tree.
	 *
	 * \param symbol The symbol to estimate the conditional probability of. A
	 * false value corresponds to \f$ \rho(0 | h) \f$ and a true value to
	 * \f$ \rho(1 | h) \f$. */
	weight_t predict(const symbol_t symbol);


	/** The estimated probability of observing a particular sequence of symbols.
	 * Given a history sequence \f$ h \f$ and a sequence of symbols \f$ y \f$,
	 * the estimated probability \f$ \rho(y | h) \f$ of observing \f$ y \f$ is
	 * \f[ \rho(y | h) = \frac{\rho(hy)}{\rho(h)} \f] where
	 * \f$ \rho(h) = P_w^\epsilon(h) \f$ is the weighted probability
	 * estimate of observing \f$ h \f$ evaluated at the root node
	 * \f$ \epsilon \f$ of the context tree.
	 *
	 * \param symbols The sequence of symbols to estimate the conditional
	 * probability of. */
	weight_t predict(symbol_list_t const& symbols);


	/** Generate a bit string of a specified length by sampling from the context
	 * tree.
	 *
	 * \param symbols Stores the generated string.
	 * \param bits The number of bits to generate. */
	void genRandomSymbols(symbol_list_t &symbols, const int bits);


	/** Generate a bit string of a specified length by sampling from the context
	 * tree and update the tree with the generated bits.
	 *
	 * \param symbols Stores the generated string.
	 * \param bits The number of bits to generate. */
	void genRandomSymbolsAndUpdate(symbol_list_t &symbols, const int bits);


	/** The logarithm of the block probability of the history sequence. */
	double logBlockProbability(void) const;


	/** \return The maximum depth of the context tree. */
	size_t depth(void) const { return m_depth; }

	/** \return The size of the stored history. */
	size_t historySize(void) const { return m_history.size(); }

	/** \return number of nodes in the context tree. */
	size_t size(void) const { return m_root ? m_root->size() : 0; }

private:

	/** Calculates which nodes in the context tree correspond to the current
	 * context and adds them to CTNode::m_context in order from root to leaf. In
	 * particular, ContextTree::m_context[0] will always correspond to the roo
	 * node and ContextTree::m_context [m_depth] corresponds to the relevant
	 * leaf node. Creates the nodes if they do not exist. */
	void updateContext(void);

	/** An array of length CTNode::m_depth + 1 used to hold the nodes in the
	 * context tree that correspond to the current context. It is important to
	 * ensure that ContextTree::updateContext() is called before accessing the
	 * contents of this array as they may otherwise be inaccurate. */
	CTNode **m_context;

	/** The agent's history. */
	symbol_list_t m_history;

	/** The root node of the context tree. */
	CTNode *m_root;

	/** The maximum depth of the context tree. */
	int m_depth;

};

#endif // __PREDICT_HPP__

#include <cassert>
#include <cmath>
#include "predict.hpp"
#include "util.hpp"


/** The value \f$\ln(0.5)\f$. This value is used often in computations and so
 * is made a constant for efficiency reasons. */
static const double log_half = std::log(0.5);

CTNode::CTNode(void) :
	m_log_kt(0.0), m_log_probability(0.0)
{
	m_count[0] = 0;
	m_count[1] = 0;
	m_child[0] = NULL;
	m_child[1] = NULL;
}


// Delete child nodes.
CTNode::~CTNode(void) {
	if (m_child[0])
		delete m_child[0];
	if (m_child[1])
		delete m_child[1];
}


// The number of descendants plus one.
int CTNode::size(void) const {
	return 1 + (child(false) ? child(false)->size() : 0) +
		(child(true) ? child(true)->size() : 0);
}


// Added to the previous logKT estimate upon observing a new symbol.
weight_t CTNode::logKTMultiplier(const symbol_t symbol) const {
	double numerator = double(m_count[symbol]) + 0.5;
	double denominator = double(visits() + 1);
	return std::log(numerator / denominator);
}


// Recalculate the log weighted probability for this node. Preconditions are:
//  * m_log_prob_est is correct.
//  * logProbWeighted() is correct for each child node.
void CTNode::updateLogProbability(void) {

	// Calculate the log weighted probability. If the current node is a leaf
	// node, this is just the KT estimate. Otherwise it is an even mixture of
	// the KT estimate and the product of the weighted probabilities of the
	// children.
	if (isLeafNode()) {
		m_log_probability = m_log_kt;
	} else {
		// The sum of the log weighted probabilities of the child nodes
		double log_child_prob = 0.0;
		log_child_prob += child(false) ? child(false)->logProbability() : 0.0;
		log_child_prob += child(true) ? child(true)->logProbability() : 0.0;

		// Calculate the log weighted probability. Use the formulation which
		// has the least chance of overflow (see function doc for details).
		double a = std::max(m_log_kt, log_child_prob);
		double b = std::min(m_log_kt, log_child_prob);
		m_log_probability = log_half + a + std::log(1.0 + std::exp(b - a));
	}
}


// Update probability estimates upon observing a new symbol.
void CTNode::update(const symbol_t symbol) {
	m_log_kt += logKTMultiplier(symbol);       // Update KT estimate
	updateLogProbability();                    // Update weighted probability
	m_count[symbol]++;                         // Update symbol counts
}


// Revert probability estimates to their most recent state.
void CTNode::revert(const symbol_t symbol) {
	m_count[symbol]--;                   // Revert symbol count
	if(m_child[symbol] && m_child[symbol]->visits() == 0) { // Delete unnecessary child node
		delete m_child[symbol];
		m_child[symbol] = NULL;
	}

	m_log_kt -= logKTMultiplier(symbol); // Revert KT estimate
	updateLogProbability();              // Revert weighted probability
}




ContextTree::ContextTree(const int depth) :
	m_root(new CTNode()), m_depth(depth)
{
	assert(depth > 0);
	m_context = new CTNode*[m_depth + 1];
	return;
}


// Delete tree and history.
ContextTree::~ContextTree(void) {
	m_history.clear();
	delete[] m_context;
	if (m_root)
		delete m_root;
}


// Clear tree and history.
void ContextTree::clear(void) {
	m_history.clear();
	if (m_root)
		delete m_root;
	m_root = new CTNode();
}


// Update the tree with a single new symbol.
void ContextTree::update(const symbol_t symbol) {

	// Traverse the tree from leaf to root according to the context. Update the
	// probabilities and symbol counts for each node.
	if (m_history.size() >= m_depth) {
		updateContext();
		for (int i = m_depth; i >= 0; i--) {
			m_context[i]->update(symbol);
		}
	}

	// Add symbol to history
	updateHistory(symbol);
}


// Update the tree with a list of observed symbols.
void ContextTree::update(symbol_list_t const& symbols) {
	symbol_list_t::const_iterator iter;
	for (iter = symbols.begin(); iter != symbols.end(); iter++) {
		update(*iter);
	}
}


// Append a symbol to history without updating context tree.
void ContextTree::updateHistory(const symbol_t symbol) {
	m_history.push_back(symbol);
}


// Append new symbols to the history.
void ContextTree::updateHistory(symbol_list_t const& symbols) {
	symbol_list_t::const_iterator iter;
	for (iter = symbols.begin(); iter != symbols.end(); iter++) {
		m_history.push_back(*iter);
	}
}


// Revert the most recent update.
void ContextTree::revert(void) {

	// No updates to revert // TODO: maybe this should be an assertion?
	if (m_history.size() == 0)
		return;

	// Get the most recent symbol and delete from history
	const symbol_t symbol = m_history.back();
	m_history.pop_back();

	// Traverse the tree from leaf to root according to the context. Update the
	// probabilities and symbol counts for each node. Delete unnecessary nodes.
	if (m_history.size() >= m_depth) {
		updateContext();
		for (int i = m_depth; i >= 0; i--) {
			m_context[i]->revert(symbol);
		}
	}
}


// Revert multiple updates
void ContextTree::revert(const int num_symbols) {
	for(int i = 0; i < num_symbols; i++) {
		revert();
	}
}


// Shrink the history without affecting the context tree
void ContextTree::revertHistory(const int num_symbols) {
	assert(0 <= num_symbols && num_symbols <= m_history.size());
	m_history.resize(m_history.size() - num_symbols);
}


// The conditional probability of symbol given the history
weight_t ContextTree::predict(const symbol_t symbol) {

	// If there is insufficient context for a prediction return 1/2.
	if (m_history.size() < m_depth) {
		return 0.5;
	}

	// Calculate the probability of the symbol s given the history h using
	// p(s | h) = p(hs) / p(h) = exp(ln p(hs) - ln p(h)).
	weight_t prob_history = logBlockProbability();
	update(symbol);
	weight_t prob_sequence = logBlockProbability();
	revert();
	return std::exp(prob_sequence - prob_history);
}


// The conditional probability of a list of symbols given the history
weight_t ContextTree::predict(symbol_list_t const& symbols) {

	// If there is insufficient context for a prediction, return the uniform
	// prediction 0.5^length
	if (m_history.size() + symbols.size() <= m_depth) {
		return pow(0.5, (int) symbols.size());
	}

	// Calculate the probability of the symbol s given the history h using
	// p(s | h) = p(hs) / p(h) = exp(ln p(hs) - ln p(h)).
	weight_t prob_history = logBlockProbability();
	update(symbols);
	weight_t prob_sequence = logBlockProbability();
	revert(symbols.size());

	return std::exp(prob_sequence - prob_history);
}

void ContextTree::genRandomSymbols(symbol_list_t &symbols, const int bits) {

	genRandomSymbolsAndUpdate(symbols, bits);
	revert(bits);
}

// generate a specified number of random symbols distributed according to
// the context tree statistics and update the context tree with the newly
// generated bits
void ContextTree::genRandomSymbolsAndUpdate(symbol_list_t &symbols, const int bits) {

	symbols.resize(bits);
	for (int i = 0; i < bits; i++) {
		symbols[i] = rand01() < predict(true);
		update(symbols[i]);
	}
}


// the logarithm of the block probability of the whole sequence
double ContextTree::logBlockProbability(void) const {
	return m_root->logProbability();
}


// Get the nodes in the current context
void ContextTree::updateContext(void) {
	assert(m_history.size() >= m_depth);

	// Traverse the tree from root to leaf according to the context. Save the
	// path taken and create new nodes as necessary.
	m_context[0] = m_root;
	CTNode **node = &m_root;
	symbol_list_t::reverse_iterator symbol_iter = m_history.rbegin();
	for (int i = 1; i <= m_depth; symbol_iter++, i++) {
		// Address of the pointer to the relevant child node
		node = &((*node)->m_child[*symbol_iter]);

		// Add node to the path (creating it if it does not exist)
		if (*node == NULL)
			*node = new CTNode();
		m_context[i] = *node;
	}
}

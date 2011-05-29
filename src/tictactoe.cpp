#include <cassert>
#include "tictactoe.hpp"
#include "util.hpp"

// Observations
const percept_t TicTacToe::oEmpty = 0;
const percept_t TicTacToe::oAgent = 1;
const percept_t TicTacToe::oEnv = 2;

// Rewards
const percept_t TicTacToe::rInvalid = 0; // -3
const percept_t TicTacToe::rLoss = 1;    // -2
const percept_t TicTacToe::rNull = 3;    //  0
const percept_t TicTacToe::rDraw = 4;    // +1
const percept_t TicTacToe::rWin = 5;     // +2

TicTacToe::TicTacToe(options_t &options) {
	m_reward = 0;
	reset();
}

void TicTacToe::reset() {
	for(int r = 0; r < 3; r++)
		for(int c = 0; c < 3; c++)
			m_board[r][c] = oEmpty;
	computeObservation();
	m_actions_since_reset = 0;
}

// Encode each square in two bits
void TicTacToe::computeObservation() {
	m_observation = 0;
	for(int r = 0; r < 3; r++)
		for(int c = 0; c < 3; c++)
			m_observation = m_board[r][c] + 4 * m_observation;
}

// True if someone has one, false otherwise
bool TicTacToe::checkWin() {
	percept_t (&b)[3][3] = m_board;
	for(int r = 0; r < 3; r++) // Check rows
		if(b[r][0] != oEmpty && b[r][0] == b[r][1] && b[r][1] == b[r][2])
			return true;
	for(int c = 0; c < 3; c++) // Check columns
		if(b[0][c] != oEmpty && b[0][c] == b[1][c] && b[1][c] == b[2][c])
			return true;
	if(b[1][1] != oEmpty && b[0][0] == b[1][1] && b[1][1] == b[2][2])
		return true; // Check diagonal
	if(b[1][1] != oEmpty && b[0][2] == b[1][1] && b[1][1] == b[2][0])
		return true; // Check diagonal
	return false; // No winner
}

// Perform agents move followed by environments move, stopping if the game ends
void TicTacToe::performAction(const action_t action) {
	assert(isValidAction(action));
	m_action = action;
	m_actions_since_reset++;

	// Decode action into desired row and column
	int r = action / 3;
	int c = action % 3;

	// If agent makes invalid move, give appropriate reward and clear board.
	if(m_board[r][c] != oEmpty) {
		m_reward = rInvalid;
		reset();
		return;
	}

	// Agent makes their move
	m_board[r][c] = oAgent;

	// If the agent wins or draws, give appropriate reward and clear board.
	if(checkWin()) {
		m_reward = rWin;
		reset();
		return;
	} else if(m_actions_since_reset == 5) {
		m_reward = rDraw;
		reset();
		return;
	}

	// Environment makes random play
	while(m_board[r][c] != oEmpty) {
		r = randRange(3);
		c = randRange(3);
	}
	m_board[r][c] = oEnv;

	// If the environment has won, give appropriate reward and clear board;
	if(checkWin()) {
		m_reward = rLoss;
		reset();
		return;
	}

	// The game continues
	m_reward = rNull;
	computeObservation();
}


std::string TicTacToe::print() const {
	std::ostringstream out;
	out << "action = " << m_action << ", observation = " << m_observation
		<< ", reward = " << m_reward << ", board:" << std::endl;
	for(int r = 0; r < 3; r++) {
		for(int c = 0; c < 3; c++){
			percept_t b = m_board[r][c];
			out << (b == oEmpty ? "." : (b == oAgent ? "A" : "O"));
		}
		out << std::endl;
	}
	return out.str();
}

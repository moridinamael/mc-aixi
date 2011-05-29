#ifndef __TICTACTOE_HPP__
#define __TICTACTOE_HPP__
#include "environment.hpp"

/** In this domain, the agent plays repeated games of TicTacToe against an
 * opponent who moves randomly. If the agent wins the game, it receives a
 * reward of 2. If there is a draw, the agent receives a reward of 1. A loss
 * penalizes the agent by -2. If the agent makes an illegal move, by moving on
 * top of an already filled square, then it receives a reward of -3. A legal
 * move that does not end the game earns no reward. An illegal reward causes
 * the game to restart.
 *
 * Domain characteristics:
 * - environment: "tictactoe"
 * - maximum action: 8 (4 bits)
 * - maximum observation: 174672 (18 bits)
 *   - 174672 (decimal) = 101010101010101010 (binary)
 * - maximum reward: 5 (3 bits)
 */
class TicTacToe : public Environment {
public:
    // set up the initial environment percept
    TicTacToe(options_t &options);

    // receives the agent's action and calculates the new environment percept
    virtual void performAction(const action_t action);


    virtual action_t maxAction() const { return 8; }
    virtual percept_t maxObservation() const { return 174762; } // 101010101010101010b
    virtual percept_t maxReward() const { return 5; }

    virtual std::string print() const;

private:
    /** Encodes the state of each square into an overall observation and saves
     * the result in TicTacToe::m_observation. Each cell corresponds to two
     * bits. */
    void computeObservation();

    /** Begin a new game. */
    void reset();

    /** Check if either player has won the game. */
    bool checkWin();

    /** Observation: empty cell. */
    static const percept_t oEmpty;

    /** Observation: cell occupied by agent. */
    static const percept_t oAgent;

    /** Observation: cell occupied by environment (opponent). */
    static const percept_t oEnv;

    /** Reward: agent performed an invalid move. */
    static const percept_t rInvalid;

    /** Reward: agent lost the game. */
    static const percept_t rLoss;

    /** Reward: agent performed a valid move, game continues. */
    static const percept_t rNull;

    /** Reward: agent drew the game. */
    static const percept_t rDraw;

    /** Reward: agent won the game. */
    static const percept_t rWin;

    /** Stores the tictactoe board. */
    percept_t m_board[3][3];

    /** The number of agent actions since the last reset. */
    int m_actions_since_reset;
};

#endif // __TICTACTOE_HPP__

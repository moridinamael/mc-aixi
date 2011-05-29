#ifndef __ROCK_PAPER_SCISSORS_HPP_
#define __ROCK_PAPER_SCISSORS_HPP_
#include "environment.hpp"

/** The agent repeatedly plays Rock-Paper-Scissor against an opponent that has
 * a slight, predictable bias in its strategy. If the opponent has won a round
 * by playing rock on the previous cycle, it will always play rock at the next
 * time step; otherwise it will pick an action uniformly at random. The agent's
 * observation is the most recently chosen action of the opponent. It receives
 * a reward of RockPaperScissors::rWin for a win, RockPaperScissors::rDraw for
 * a draw and RockPaperScissors::rLose for a loss.
 *
 * Domain characteristics:
 *  - environment: "rock-paper-scissors"
 *  - maximum action: 2 (2 bits)
 *  - maximum observation: 2 (2 bits)
 *  - maximum reward: 2 (2 bits) */
class RockPaperScissors: public Environment{
public:
	RockPaperScissors(options_t &options);

	virtual void performAction(const action_t action); // paper = 0 scissor = 1 rock = 2

	virtual action_t maxAction() const { return 2; }
	virtual percept_t maxObservation() const { return 2; }
	virtual percept_t maxReward() const { return 2; }

	virtual std::string print() const;

private:
	static const percept_t oRock;
	static const percept_t oPaper;
	static const percept_t oScissors;

	static const percept_t rLose;
	static const percept_t rDraw;
	static const percept_t rWin;

	static const action_t aRock;
	static const action_t aPaper;
	static const action_t aScissors;
};

#endif // __ROCK_PAPER_SCISSORS_HPP_







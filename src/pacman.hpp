#ifndef __PACMAN_HPP__
#define __PACMAN_HPP__

#include <string>
#include "environment.hpp"


/** This domain is a partially observable version of the classic PacMan game.
 * The agent must navigate a 17x17 maze and eat the food pellets that are
 * distributed across the maze. Four ghosts roam the maze. They move initially
 * at random, until there is a Manhattan distance of 5 between them and PacMan,
 * whereupon they will aggressively pursue PacMan for a short duration. The
 * maze structure and game are the same as the original arcade game, however
 * the PacMan agent is hampered by partial observability. PacMan is unaware of
 * the maze structure and only receives a 4-bit observation describing the wall
 * configuration at its current location. It also does not know the exact
 * location of the ghosts, receiving only 4-bit observations indicating whethe
 * a ghost is visible (via direct line of sight) in each of the four cardinal
 * directions. In addition, the location of the food pellets is unknown except
 * for a 3-bit observation that indicates whether food can be smelt within a
 * Manhattan distance of 2, 3 or 4 from PacMan's location, and another 4-bit
 * observation indicating whether there is food in its direct line of sight.
 * A final single bit indicates whether PacMan is under the effects of a power
 * pill. At the start of each episode, a food pellet is placed down with
 * probability 0.5 at every empty location on the grid. The agent receives a
 * penalty of 1 for each movement action, a penalty of 10 for running into a
 * wall, a reward of 10 for each food pellet eaten, a penalty of 50 if it is
 * caught by a ghost, and a reward of 100 for collecting all the food.
 * If multiple such events occur, then the total reward is cumulative, i.e.
 * running into a wall and being caught would give a penalty of 60. The episode
 * resets if the agent is caught or if it collects all the food.
 *
 * Domain characteristics:
 * - maximum action: 3
 * - observation bits: 16
 * - reward bits: 8
 */
class PacMan : public Environment {
public:

	// set up the initial environment percept
	PacMan(options_t &options);

	// receives the agent's action and calculates the new environment percept
	virtual void performAction(action_t action);
	virtual std::string print() const;
	
	virtual action_t maxAction() const { return 4; }
	virtual percept_t maxObservation() const { return (1 << 16) - 1 ; }
	virtual percept_t maxReward() const { return 161; } // TODO: check

private:
	int pacmanX; // PacMan's X coordinate
	int pacmanY; // PacMan's Y coordinate
	
	int aGhostX; // First ghost's X coordinate
	int aGhostY; // First ghost's Y coordinate
	
	int bGhostX; // Second ghost's X coordinate
	int bGhostY; // Second ghost's Y coordinate
	
	int cGhostX; // Third ghost's X coordinate
	int cGhostY; // Third ghost's Y coordinate
	
	int dGhostX; // Fourth ghost's X coordinate
	int dGhostY; // Fourth ghost's Y coordinate
	
	//Amount of time that they see Pacman and agressivley pursue him.
	int sniffA;
	int sniffB;
	int sniffC;
	int sniffD;
	
	bool poweredUp; //Flag whether Pacman is under the effects of a power pellet.
	int powerLeft;  //How much timesteps left for Pacman to have power
	
	// Game map
	std::string map[19];

	//binary observation
	bool binaryObservation[16];
	
	//Number of timesteps
	int timestep;
	
	//Nubmer of resets
	int resets;
	
	//Number of pellets still remaining to be eaten
	int pelletCount;
	
	//Flags on whether a ghost is currently covering a pellet
	//So that we can revert the map point after the ghost moves
	// 0 - covers nothing, 1 - covers a pellet, 2 - covers a power up
	int aGhostCovering;
	int bGhostCovering;
	int cGhostCovering;
	int dGhostCovering;
	
	bool reset;
	
	void movePacmanAndUpdateReward(int x, int y);
	void moveGhostAndUpdateReward(int *ghostX, int *ghostY, int *sniff, int *covers, char ghost);
	void ghostPursuitMove(int *ghostX, int *ghostY, int *covers, char ghost);
	bool isValidGhostMove(int x, int y);
	void ghostRandomMove(int *ghostX, int *ghostY, int *covers, char ghost);
	int binaryToDecimal(bool binary[]);
	bool findPacman(int *ghostX, int *ghostY, int *sniff);
	void updateObservation();
	int manhattanDistance(int x1, int y1, int x2, int y2);
	void resetEpisode();
	void resetGhost(char ghost);
};

#endif // __PACMAN_HPP__

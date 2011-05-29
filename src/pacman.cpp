#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include "pacman.hpp"
#include "util.hpp"

PacMan::PacMan(options_t &options) {
	map[0] = "                 ";
	map[1] = " ** *** * *** ** ";
	map[2] = "O               O";
	map[3] = " ** * ***** * ** ";
	map[4] = "    *   *   *    ";
	map[5] = "*** *** * *** ***";
	map[6] = "*** *       * ***";
	map[7] = "*** * * AB* * ***";
	map[8] = "x   * * CD* *   x";
	map[9] = "*** * ***** * ***";
	map[10] = "*** *       * ***";
	map[11] = "*** * ***** * ***";
	map[12] = "        P        ";
	map[13] = " ** *** * *** ** ";
	map[14] = "O *           * O";
	map[15] = "* * * ***** * * *";
	map[16] = "    *   *   *    ";
	map[17] = " ****** * ****** ";
	map[18] = "                 ";

	resets = 0;
	resetEpisode();

	timestep = 0;
	m_reward = 0;

}

void PacMan::performAction(action_t action) {
	timestep++;

	int newX, newY;

	m_reward = 200;

	if (action == 0) {//Go up
		newX = pacmanX;
		newY = pacmanY - 1;
	} else if (action == 1) { //Go right
		newX = pacmanX + 1;
		newY = pacmanY;
	} else if (action == 2) { //Go down
		newX = pacmanX;
		newY = pacmanY + 1;
	} else if (action == 3) { // Go left
		newX = pacmanX - 1;
		newY = pacmanY;
	}

	movePacmanAndUpdateReward(newX, newY);

	moveGhostAndUpdateReward(&aGhostX, &aGhostY, &sniffA, &aGhostCovering, 'A');
	moveGhostAndUpdateReward(&bGhostX, &bGhostY, &sniffB, &bGhostCovering, 'B');
	moveGhostAndUpdateReward(&cGhostX, &cGhostY, &sniffC, &cGhostCovering, 'C');
	moveGhostAndUpdateReward(&dGhostX, &dGhostY, &sniffD, &dGhostCovering, 'D');

	updateObservation();
	m_action = action;

	if (reset) {
		resetEpisode();
	}

	//printMap();
}

/*Pacman moves to a valid location and we update the reward system.*/
void PacMan::movePacmanAndUpdateReward(int x, int y) {
	//-1 per timestep
	m_reward -= 1;
	bool validMove = false;

	//Warp zone. Move from one x to another x.
	if ((x < 0 || x > 16) && (y == 8)) {
		if (x < 0) {
			x = 16;
		} else if (x > 16) {
			x = 0;
		}

		validMove = true;
	}
	//Check that it's a Valid move and it ain't a wall
	else if (x < 0 || x > 16 || y < 0 || y > 18 || map[y][x] == '*') {

		m_reward -= 10;
		validMove = false;
	}
	//Moved into a ghost.
	else if (map[y][x] == 'A' || map[y][x] == 'B' || map[y][x] == 'C'
			|| map[y][x] == 'D') {

		//Got power. Frak up the ghost
		if (poweredUp) {
			//m_reward += 50;
			validMove = false;
			resetGhost(map[y][x]);
		}
		//Ghost ate us. Ooops. Reset to a new episode
		else {
			m_reward -= 50;

			validMove = false;
			reset = true;
		}
	}
	//Got a pellet. Woot!
	else if (map[y][x] == '.') {
		m_reward += 10;

		//Woot! Game winner
		if (--pelletCount == 0) {
			std::cout << "GAME WINNER!\n";
			m_reward += 100;
			reset = true;
		}

		validMove = true;
	}
	//POWER OVERWHELMING!!!
	else if (map[y][x] == 'O') {
		poweredUp = true;
		powerLeft = 5;

		validMove = true;
	} else if (map[y][x] == ' ' || map[y][x] == 'x') {
		validMove = true;
	} else {
		//Should never get get here.
		std::cout << "Frak: " << map[y][x] << "\n";
	}

	//Update map
	if (validMove) {
		if ((pacmanX == 0 || pacmanX == 16) && (pacmanY == 8)) {
			map[pacmanY][pacmanX] = 'x';
		} else {
			map[pacmanY][pacmanX] = ' ';
		}

		map[y][x] = 'P';

		pacmanX = x;
		pacmanY = y;
	}

	// Power runs out
	if (poweredUp) {
		poweredUp = --powerLeft > 0;
	}
}

void PacMan::moveGhostAndUpdateReward(int *ghostX, int *ghostY, int *sniff,
		int *covers, char ghost) {
	// We do sniff > -2 because we don't want the ghost to recontinue 
	// chasing pacman right away. Give pacman a breather of 3 timesteps.
	if (findPacman(ghostX, ghostY, sniff) && *sniff > -2) {
		ghostPursuitMove(ghostX, ghostY, covers, ghost);
		*sniff--;
	} else {
		ghostRandomMove(ghostX, ghostY, covers, ghost);
		*sniff--;
	}
}

void PacMan::ghostRandomMove(int *ghostX, int *ghostY, int *covers, char ghost) {
	bool validMove = false;

	int newX, newY;

	bool triedUp = false;
	bool triedRight = false;
	bool triedDown = false;
	bool triedLeft = false;

	// Loop until random gives us a valid move for a ghost
	while (!validMove) {
		int action = randRange(4);

		if (action == 0) { //Go up
			newX = *ghostX;
			newY = (*ghostY) - 1;
			triedUp = true;
		} else if (action == 1) { //Go right
			newX = (*ghostX) + 1;
			newY = *ghostY;
			triedRight = true;
		} else if (action == 2) { //Go down
			newX = *ghostX;
			newY = (*ghostY) + 1;
			triedDown = true;
		} else if (action == 3) { // Go Left
			newX = (*ghostX) - 1;
			newY = *ghostY;
			triedLeft = true;
		}

		if (newX == pacmanX && newY == pacmanY) {
			if (poweredUp) { //Ghost made itself be eaten
				//m_reward += 50;
				resetGhost(ghost);
			} else {
				//Ghost ate Pacman, reset episode
				m_reward -= 50;
				reset = true;
			}

			return;
		}
		//Ghost can move to this new spot
		else if (isValidGhostMove(newX, newY)) {
			validMove = true;
		}
		//No valid move available. Nevermind.
		else if (!validMove && triedUp && triedRight && triedDown && triedLeft) {
			return;
		}
	}

	//If ghost was covering something, put it back into the world
	if (*covers == 0) {
		map[*ghostY][*ghostX] = ' ';
	} else if (*covers == 1) {
		map[*ghostY][*ghostX] = '.';
	} else if (*covers == 2) {
		map[*ghostY][*ghostX] = 'O';
	} else if (*covers == 3) {
		map[*ghostY][*ghostX] = 'x';
	} else {
		//assert(false);
	}

	//If ghost is going to cover something in the new space, save it for now.
	if (map[newY][newX] == ' ') {
		*covers = 0;
	} else if (map[newY][newX] == '.') {
		*covers = 1;
	} else if (map[newY][newX] == 'O') {
		*covers = 2;
	} else if (map[newY][newX] == 'x') {
		*covers = 3;
	} else {
		//assert(false);
	}

	*ghostX = newX;
	*ghostY = newY;

	map[newY][newX] = ghost;

}

void PacMan::ghostPursuitMove(int *ghostX, int *ghostY, int *covers, char ghost) {
	int newX, newY;

	int currentDistance = manhattanDistance(pacmanX, pacmanY, *ghostX, *ghostY);

	bool pursuing = false;

	//Go up
	int upX = *ghostX;
	int upY = (*ghostY) - 1;
	if (isValidGhostMove(upX, upY) && manhattanDistance(pacmanX, pacmanY, upX,
			upY) < currentDistance) {
		pursuing = true;
		newX = upX;
		newY = upY;
	}

	//Go right
	int rightX = (*ghostX) + 1;
	int rightY = *ghostY;
	if (isValidGhostMove(rightX, rightY) && manhattanDistance(pacmanX, pacmanY,
			rightX, rightY) < currentDistance) {
		pursuing = true;
		newX = rightX;
		newY = rightY;
	}

	//Go down
	int downX = *ghostX;
	int downY = (*ghostY) + 1;
	if (isValidGhostMove(downX, downY) && manhattanDistance(pacmanX, pacmanY,
			downX, downY) < currentDistance) {
		pursuing = true;
		newX = downX;
		newY = downY;
	}
	// Go Left
	int leftX = (*ghostX) - 1;
	int leftY = *ghostY;
	if (isValidGhostMove(leftX, leftY) && manhattanDistance(pacmanX, pacmanY,
			leftX, leftY) < currentDistance) {
		pursuing = true;
		newX = leftX;
		newY = leftY;
	}

	//Pursuit move not valid (wall?) Get a random move instead.
	if (!pursuing) {
		ghostRandomMove(ghostX, ghostY, covers, ghost);
		return;
	}

	//Ghost got pacman. But it shouldn't move
	if (newX == pacmanX && newY == pacmanY) {
		if (poweredUp) {
			//m_reward += 50;
			resetGhost(ghost);
		} else {
			m_reward -= 50;
			reset = true;
		}
		return;
	}

	//If ghost covered something, put it back in.
	if (*covers == 0) {
		map[*ghostY][*ghostX] = ' ';
	} else if (*covers == 1) {
		map[*ghostY][*ghostX] = '.';
	} else if (*covers == 2) {
		map[*ghostY][*ghostX] = 'O';
	} else if (*covers == 3) {
		map[*ghostY][*ghostX] = 'x';
	} else {
		//Should not get here unless P=NP. Uh, right.
		std::cout << "WTF?\n";
	}

	//If ghost is gonna cover something, save it somewhere first
	if (map[newY][newX] == ' ') {
		*covers = 0;
	} else if (map[newY][newX] == '.') {
		*covers = 1;
	} else if (map[newY][newX] == 'O') {
		*covers = 2;
	} else if (map[newY][newX] == 'x') {
		*covers = 3;
	} else {
		//Should not get here unles 2+2=5.
		//For extremely large values of 2.
		std::cout << "Pakshit\n";
	}

	*ghostX = newX;
	*ghostY = newY;

	map[newY][newX] = ghost;
}

bool PacMan::findPacman(int *ghostX, int *ghostY, int *sniff) {
	//Ghosts pursue pacman for 5 timesteps or less.
	if (*sniff > 0) {
		return true;
	} else if (manhattanDistance(pacmanX, pacmanY, *ghostX, *ghostY) <= 5) {
		*sniff = 5;
		return true;
	} else {
		return false;
	}
}

//Update the observation we return to the agent.
//Make binary array that represents a 16-bit value.
void PacMan::updateObservation() {
	//bool binary[16];
	binaryObservation[0] = false;
	binaryObservation[1] = false;
	binaryObservation[2] = false;
	binaryObservation[3] = false;
	binaryObservation[4] = false;
	binaryObservation[5] = false;
	binaryObservation[6] = false;
	binaryObservation[7] = false;
	binaryObservation[8] = false;
	binaryObservation[9] = false;
	binaryObservation[10] = false;
	binaryObservation[11] = false;
	binaryObservation[12] = false;
	binaryObservation[13] = false;
	binaryObservation[14] = false;
	binaryObservation[15] = false;

	//Get the surrounding spaces around pacman
	if (pacmanY - 1 < 0 || map[pacmanY - 1][pacmanX] == '*') {
		binaryObservation[0] = true;
	}

	if (pacmanX + 1 > 16 || map[pacmanY][pacmanX + 1] == '*') {
		binaryObservation[1] = true;
	}

	if (pacmanY + 1 > 18 || map[pacmanY + 1][pacmanX] == '*') {
		binaryObservation[2] = true;
	}

	if (pacmanX - 1 < 0 || map[pacmanY][pacmanX] == '*') {
		binaryObservation[3] = true;
	}

	//See if pacman can find a ghost or a pellet north
	int upY = pacmanY - 1;
	while (upY > 0 && map[upY][pacmanX] != '*') {
		if (map[upY][pacmanX] == 'A' || map[upY][pacmanX] == 'B'
				|| map[upY][pacmanX] == 'C' || map[upY][pacmanX] == 'D') {

			binaryObservation[4] = true;
		}

		if (map[upY][pacmanX] == '.') {
			binaryObservation[11] = true;
		}

		upY--;
	}

	//See if pacman can find a ghost or a pellet east.
	int rightX = pacmanX + 1;
	while (rightX < 17 && map[pacmanY][rightX] != '*') {
		if (map[pacmanY][rightX] == 'A' || map[pacmanY][rightX] == 'B'
				|| map[pacmanY][rightX] == 'C' || map[pacmanY][rightX] == 'D') {

			binaryObservation[5] = true;
		}

		if (map[pacmanY][rightX] == '.') {
			binaryObservation[12] = true;
		}

		rightX++;
	}

	//See if pacman can find a ghost or a pellet south.
	int downY = pacmanY + 1;
	while (downY < 19 && map[downY][pacmanX] != '*') {

		if (map[downY][pacmanX] == 'A' || map[downY][pacmanX] == 'B'
				|| map[downY][pacmanX] == 'C' || map[downY][pacmanX] == 'D') {

			binaryObservation[6] = true;
		}
		if (map[downY][pacmanX] == '.') {
			binaryObservation[13] = true;
		}
		downY++;
	}

	//See if pacman can find a ghost or a pellet east.
	int leftX = pacmanX - 1;
	while (leftX > 0 && map[pacmanY][leftX] != '*') {
		if (map[pacmanY][leftX] == 'A' || map[pacmanY][leftX] == 'B'
				|| map[pacmanY][leftX] == 'C' || map[pacmanY][leftX] == 'D') {

			binaryObservation[7] = true;
		}

		if (map[pacmanY][leftX] == '.') {
			binaryObservation[14] = true;
		}

		leftX--;
	}

	//Check if there's a pellet nearby
	for (int y = 0; y < 19; y++) {
		for (int x = 0; x < 17; x++) {
			int distance = manhattanDistance(pacmanX, pacmanY, x, y);

			if (!binaryObservation[8] && distance <= 2 && map[y][x] == '.') {
				binaryObservation[8] = true;
			}

			if (!binaryObservation[9] && distance <= 3 && map[y][x] == '.') {
				binaryObservation[9] = true;
			}

			if (!binaryObservation[10] && distance <= 4 && map[y][x] == '.') {
				binaryObservation[10] = true;
			}
		}
	}

	//See if pacman is under the effects of a power pellet
	binaryObservation[15] = poweredUp;

	m_observation = binaryToDecimal(binaryObservation);
}

// If it's not a wall or another ghost and within the world bounds
// then it's a valid move.
bool PacMan::isValidGhostMove(int x, int y) {
	if (x < 0 || x > 16 || y < 0 || y > 18 || map[y][x] == '*' || map[y][x]
			== 'A' || map[y][x] == 'B' || map[y][x] == 'C' || map[y][x] == 'D') {

		return false;
	}

	return true;
}

void PacMan::resetEpisode() {
	resets++;

	if (resets > 1) {
		map[pacmanY][pacmanX] = ' ';
		map[aGhostY][aGhostX] = ' ';
		map[bGhostY][bGhostX] = ' ';
		map[cGhostY][cGhostX] = ' ';
		map[dGhostY][dGhostX] = ' ';
	}
	map[2][0] = 'O';
	map[2][16] = 'O';
	map[14][0] = 'O';
	map[14][16] = 'O';

	pacmanX = 8;
	pacmanY = 12;

	aGhostX = 8;
	aGhostY = 7;

	bGhostX = 9;
	bGhostY = 7;

	cGhostX = 8;
	cGhostY = 8;

	dGhostX = 9;
	dGhostY = 8;

	map[pacmanY][pacmanX] = 'P';
	map[aGhostY][aGhostX] = 'A';
	map[bGhostY][bGhostX] = 'B';
	map[cGhostY][cGhostX] = 'C';
	map[dGhostY][dGhostX] = 'D';

	pelletCount = 0;
	for (int x = 0; x < 19; x++) {
		for (int y = 0; y < 17; y++) {
			if ((y == 0 || y == 16) && (x == 8)) {
				map[x][y] = 'x';
			}

			if (map[x][y] == '.') {
				map[x][y] = ' ';
			}

			if (map[x][y] == ' ' && randRange(2)) {
				map[x][y] = '.';
				pelletCount++;
			}
		}
	}

	sniffA = 0;
	sniffB = 0;
	sniffC = 0;
	sniffD = 0;

	poweredUp = false;
	powerLeft = 0;

	aGhostCovering = 0;
	bGhostCovering = 0;
	cGhostCovering = 0;
	dGhostCovering = 0;

	m_observation = 0;

	reset = false;
}

void PacMan::resetGhost(char ghost) {
	std::cout << "Ghost " << ghost << " eaten\n";
	int cover;
	int oldX;
	int oldY;

	switch (ghost) {
	case 'A':
		sniffA = 0;
		cover = aGhostCovering;
		aGhostCovering = 0;

		oldX = aGhostX;
		oldY = aGhostY;
		aGhostX = 8;
		aGhostY = 7;
		map[7][8] = 'A';
		break;
	case 'B':
		sniffB = 0;
		cover = bGhostCovering;
		bGhostCovering = 0;

		oldX = bGhostX;
		oldY = bGhostY;
		bGhostX = 9;
		bGhostY = 7;
		map[7][9] = 'B';
		break;
	case 'C':
		sniffC = 0;
		cover = cGhostCovering;
		cGhostCovering = 0;

		oldX = cGhostX;
		oldY = cGhostY;
		cGhostX = 8;
		cGhostY = 8;
		map[8][8] = 'C';
		break;
	case 'D':
		sniffD = 0;
		cover = dGhostCovering;
		dGhostCovering = 0;

		oldX = dGhostX;
		oldY = dGhostY;
		dGhostX = 9;
		dGhostY = 8;

		map[8][9] = 'D';
		break;
	default:
		std::cout << "Crap\n";
		break;
	}

	if (cover == 0) {
		map[oldY][oldX] = ' ';
	} else if (cover == 1) {
		map[oldY][oldX] = '.';
	} else if (cover == 2) {
		map[oldY][oldX] = 'O';
	} else {
		std::cout << "WTF?\n";
	}
}

/*
 * Rest of the functions below are utility functions
 */

int PacMan::binaryToDecimal(bool binary[]) {
	int dec = 0;

	for (int x = 15; x >= 0; x--) {
		if (binary[x]) {
			dec += 1 << (15 - x);
			//dec += pow(2, (15 - x));
		}
	}

	return dec;
}

int PacMan::manhattanDistance(int x1, int y1, int x2, int y2) {
	return abs(x1 - x2) + abs(y1 - y2);
}

std::string PacMan::print() const {
	std::ostringstream out;
	out << "Timestep: " << timestep << " Resets: " << resets << "\n";
	for (int x = 0; x < 19; x++) {
		out << map[x] << "\n";
	}
	return out.str();
}

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <sstream>
#include "maze.hpp"

// Actions
const action_t Maze::aLeft = 0;
const action_t Maze::aUp = 1;
const action_t Maze::aRight = 2;
const action_t Maze::aDown = 3;

// Observations (uninformative encoding)
const percept_t Maze::oNull = 0;

// Observations (wall encoding)
const percept_t Maze::oLeftWall = 1;
const percept_t Maze::oUpWall = 2;
const percept_t Maze::oRightWall = 4;
const percept_t Maze::oDownWall = 8;

// Maze layout constants
const char Maze::cWall = '@';
const char Maze::cTeleportTo = '*';
const char Maze::cTeleportFrom = '!';
const char Maze::cEmpty = '&';


Maze::Maze(options_t &options) {
	// Configure the environment based on the options.
	configure(options);

	// Assign the agent randomly
	teleportAgent();

	// Initial reward and percept
	m_reward = 0;
	calculateObservation();
}


// Free memory
Maze::~Maze() {
	for(int r = 0; r < m_num_rows; r++) {
		delete[] m_maze_layout[r];
		delete[] m_maze_rewards[r];
	}
	delete[] m_maze_layout;
	delete[] m_maze_rewards;
}


void Maze::configure(options_t &options) {
	// Get the dimensions of the maze
	getRequiredOption(options, "maze-num-rows", m_num_rows);
	getRequiredOption(options, "maze-num-cols", m_num_cols);
	assert(m_num_rows > 0);
	assert(m_num_cols > 0);

	// Get the type of observations to give
	std::string encoding;
	getOption(options, "maze-observation-encoding",
	          std::string("uninformative"), encoding);
	if(encoding == "uninformative"){
		m_obs_encoding = cUninformative;
	} else if(encoding == "walls"){
		m_obs_encoding = cWalls;
	} else if(encoding == "coordinates"){
		m_obs_encoding = cCoordinates;
	} else {
		// Error (unknown observation)
		std::cerr << "ERROR: Unknown observation encoding: '" << encoding
			<< "'" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Used to check if there are any squares which can be teleported to.
	bool teleport_impossible = true;

	// Used to record the minimum and maximum rewards in the maze.
	int min_reward = std::numeric_limits<int>::max();
	m_max_reward = std::numeric_limits<int>::min();

	// Allocate and parse maze, determine if teleportation is possible and the
	// range of rewards.
	m_maze_rewards = new int*[m_num_rows];
	m_maze_layout = new char*[m_num_cols];
	for(int r = 0; r < m_num_rows; r++) {
		// Allocate space for row
		m_maze_layout[r] = new char[m_num_cols];
		m_maze_rewards[r] = new int[m_num_cols];

		// Get the reward string for the current row
		std::stringstream rewards;
		rewards << "maze-rewards" << r + 1;
		requiredOption(options, rewards.str());
		rewards.str(options[rewards.str()]);

		// Get the layout string for the current row
		std::stringstream layout;
		layout << "maze-layout" << r + 1;
		requiredOption(options, layout.str());
		layout.str(options[layout.str()]);

		// Parse the reward and layout strings. Record appropriate properties
		// of the maze.
		for(int c = 0; c < m_num_cols; c++) {
			layout >> m_maze_layout[r][c];
			rewards >> m_maze_rewards[r][c];
			rewards.ignore(1, ','); // Ignore next character

			if(m_maze_layout[r][c] == cTeleportTo)
				teleport_impossible = false;

			min_reward = std::min(min_reward, m_maze_rewards[r][c]);
			m_max_reward = std::max(m_max_reward, m_maze_rewards[r][c]);
		}
	}

	// Causes an error if it is impossible for the agent to teleport anywhere.
	if(teleport_impossible) {
		std::cerr << "ERROR (Maze): There must be at least one square the agent"
			<< " can teleport to." << std::endl;
		exit(EXIT_FAILURE);
	}

	// Adjust rewards so they begin at 0.
	m_max_reward -= min_reward;
	for(int r = 0; r < m_num_rows; r++)
		for(int c = 0; c < m_num_cols; c++)
			m_maze_rewards[r][c] -= min_reward;
}

void Maze::teleportAgent() {
	m_teleported = true;
	do {
		m_row = randRange(m_num_rows);
		m_col = randRange(m_num_cols);
	} while(m_maze_layout[m_row][m_col] != cTeleportTo);
}


percept_t Maze::maxObservation() const {
	if(m_obs_encoding == cUninformative) {
		// Only one observation
		return oNull;
	} else if(m_obs_encoding == cWalls) {
		// Maximum observation is walls on all sides
		return aLeft + aUp + aRight + aDown;
	} else if(m_obs_encoding == cCoordinates) {
		// Maximum observation is square at intersection of last row and column
		return m_num_rows * m_num_cols - 1;
	}
}


void Maze::calculateObservation() {
	if(m_obs_encoding == cUninformative) {
		// Uninformative observation: agent always receives same observation
		m_observation = oNull;
	} else if(m_obs_encoding == cWalls) {
		// Agent observes adjacent walls.
		m_observation = 0;
		if(m_col == 0 || m_maze_layout[m_row][m_col - 1] == cWall)
			m_observation += oLeftWall;
		if(m_row == 0 || m_maze_layout[m_row - 1][m_col] == cWall)
			m_observation += oUpWall;
		if(m_col + 1 == m_num_cols || m_maze_layout[m_row][m_col + 1] == cWall)
			m_observation += oRightWall;
		if(m_row + 1 == m_num_rows || m_maze_layout[m_row + 1][m_col] == cWall)
			m_observation += oDownWall;
	} else if(m_obs_encoding == cCoordinates) {
		// Agent observes the coordinates of its current square.
		m_observation = m_row * m_num_cols + m_col;
	}
}

void Maze::performAction(action_t action) {
	assert(isValidAction(action));
	m_action = action;

	m_teleported = false;
	m_wall_collision = false;

	// Calculate the square the agent is attempting to move to, making sure they
	// don't move outside the maze.
	int m_row_to = (action == aUp ? -1 : 0) + (action == aDown ? 1 : 0);
	m_row_to = std::min(std::max(m_row_to + m_row, 0), m_num_rows - 1);
	int m_col_to = (action == aLeft ? -1 : 0) + (action == aRight ? 1 : 0);
	m_col_to = std::min(std::max(m_col_to + m_col, 0), m_num_cols - 1);

	// Move the agent, making sure they don't walk into a wall.
	m_wall_collision = m_maze_layout[m_row_to][m_col_to] == cWall;
	if(!m_wall_collision) {
		m_row = m_row_to;
		m_col = m_col_to;
	}

	// Teleport if appropriate
	if(m_maze_layout[m_row][m_col] == cTeleportFrom)
		teleportAgent();

	// Calculate the reward for the square the agent *attempted* to move into,
	// regardless of whether they were able to move into it.
	m_reward = m_maze_rewards[m_row_to][m_col_to];

	// Calculate the observation for the square the agent is now in. That is,
	// after any movement or teleportation has occurred.
	calculateObservation();
}


std::string Maze::print() const {
	std::ostringstream out;
	out << "row = " << m_row << ", col = " << m_col << ", observation = "
		<< m_observation << ", reward = " << m_reward
		<< (m_teleported ? ", teleported" : "")
		<< (m_wall_collision ? ", wall collision" : "") << std::endl;
	for(int r = 0; r < m_num_rows; r++) {
		for(int c = 0; c < m_num_cols; c++) {
			if(m_row == r && m_col == c) {
				out << "A";
			} else {
				out << m_maze_layout[r][c];
			}
		}
		out << std::endl;
	}
	return out.str();
}

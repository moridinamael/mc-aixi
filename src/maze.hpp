#ifndef __MAZE_HPP__
#define __MAZE_HPP__
#include "environment.hpp"

/** A two-dimensional maze environment. The agent is able to move through the
 * maze in each of the four cardinal directions (Maze::aLeft, Maze::aUp,
 * Maze::aRight, Maze::aDown).The user is able to specify (via a
 * connfiguration file) the dimensions, layout, and rewards of the maze as well
 * as the type of observations given to the agent. In particular, the maze
 * is a certain number of rows high and columns wide, each cell in the maze is
 * of a certain type and has a certain reward.
 *
 * The type of each cell determines
 * what happens to the agent when it attempts to move into the cell:
 * - Wall (Maze::cWall): represents an impassable cell, attempting to move into
 *   a wall will result in the agent remaining at it's current position.
 * - Empty (Maze::cEmpty): an empty cell through which the agent can freely
 *   pass.
 * - Teleport from (Maze::cTeleportFrom): represents a cell which, upon entry,
 *   will randomly teleport the agent to another cell of type Maze::cTeleportTo.
 * - Teleport to (Maze::cTeleportTo): A cell which can be teleported to.
 *   Otherwise, the cell acts identically to an empty cell. The maze MUST
 *   contain at least one of these cells.
 *
 * The reward for each cell describes the reward received by the agent when it
 * attempts to move into that cell. For example, when the agent attempts to move
 * into a wall square, it gets the reward from the wall cell despite the fact
 * that it does not end its turn in that cell. Similarly, when the agent moves
 * into a Maze::cTeleportFrom cell it gets the reward from that cell rather
 * than from the cell it will be teleported to.
 *
 * Finally, the user can choose one of several observation encodings to give
 * to the agent:
 * - Uninformative (Maze::cUninformative): The observation is a single
 *   unchanging value.
 * - Walls (Maze::cWalls): The observation encodes the presence of walls in
 *   adjacent squares. This is encoded as a sum of the flags: Maze::oDownWall,
 *   Maze::oLeftWall, Maze::oRightWall, Maze::oUpWall, each of which indicates
 *   the presence of a wall in the corresponding direction.
 * - Coordinates (Maze::cCoordinates): The observation gives the coordinates
 *   of the cell occupied by the agent. This is encoded as row * num_cols + col.
 *
 * Domain characteristic:
 * - environment: "maze"
 * - maximum action: 3 (2 bits)
 * - maximum observation:
 *   - Uninformative: 0 (1 bit)
 *   - Walls: 15 (4 bits)
 *   - Coordinates: Maze::m_num_rows * Maze::m_num_cols - 1
 * - maximum reward: maximum value in maze rewards (Maze::m_maze_rewards)
 *
 * Configuration options:
 * - maze-num-rows: the number of rows in the maze.
 * - maze-num-cols: the number of columns in the maze.
 * - maze-rewards#: comma-separated list of rewards for each square in row #. If
 *   the agent enters (or attempts to enter) a particular square it receives
 *   the corresponding reward. 1 <= # <= maze-num-rows.
 * - maze-layout#: The layout of row # of the maze (1 <= # <= maze-num-rows).
 *   Contains maze-num-cols symbols as follows:
 *   - Maze::cWall: indicates the square cannot be entered (a.k.a. "a wall").
 *   - Maze::cTeleportTo: indicates the square can be entered and teleported to.
 *   - Maze::cEmpty: indicates the square can be entered but not
 *     teleported to.
 *   - Maze::cTeleportFrom: indicates the square can be entered, and that doing
 *     so will randomly teleport the agent to a Maze::cTeleportTo square before
 *     the next turn (the agent receives the reward before the teleportation
 *     occurs).
 * - maze-observation-encoding: Specifies the type of observations the agent recieves
 *   - uninformative: the agent receives the same observation each cycle.
 *   - walls: the agent receives an observation specifying whether there are
 *            walls "@" above, below, left, or right of its current position.
 *   - coordinates: the observation specifies the coordinates of the agent in
 *                  the maze. */
class Maze: public Environment {
public:
	Maze(options_t &options);

	/** Frees memory allocated to store Maze::m_maze_rewards and
	 * Maze::m_maze_layout. */
	~Maze();

	virtual action_t maxAction() const { return 3; }

	/** The maximum observation that can be given to the agent. Depends on
	 * the observation type (Maze::m_obs_type) and (potentially) the dimensions
	 * of the maze (Maze::m_num_rows and Maze::m_num_cols). */
	virtual percept_t maxObservation() const;

	/** The maximum reward that can be received by the agent. This is simply
	 * the maximum value in array Maze::m_maze_rewards (cached in
	 * Maze::m_max_reward). */
	virtual percept_t maxReward() const { return m_max_reward; }

	virtual void performAction(const action_t action);

	/** Print the current state of the environment, including the current
	 * location, observation, reward, and maze layout. */
	virtual std::string print() const;

private:
	/** Configure the maze based on the configuration options. May exit if the
	 * configuration is not validly formatted. */
	void configure(options_t &options);

	/** Randomly place the agent at any Maze::cTeleportTo location. */
	void teleportAgent();

	/** Determine the observation to give to the agent based on its current
	 * location (Maze::m_row and Maze::m_col) and the observation type
	 * (Maze::m_obs_type). Store the observation in Maze::m_observation. */
	void calculateObservation();


	/** The number of rows in the maze. Set via required configuration option
	 * "maze-num-rows". */
	int m_num_rows;

	/** The number of columns in the maze. Set via required configuration
	 * option "maze-num-cols". */
	int m_num_cols;

	/** Stores the reward for each square in the maze. In particular,
	 * Maze::m_maze_rewards[r][c] is the reward given to the agent when it
	 * attempts to move into the square at the intersection of row r and
	 * column c (regardless of whether it is actually able to move into the
	 * desired square). All rewards are nonnegative. Row # of the reward
	 * array is set via the (mandatory) configuration option "maze-rewards#"
	 * (where # is replaced with the row number). */
	int **m_maze_rewards;

	/** Stores the layout of the maze. In particular, Maze::m_maze_layout[r][c]
	 * specifies the type of square in row r and column c. Different types of
	 * squares include Maze::cWall, Maze::cTeleportTo, Maze::cTeleportFrom and
	 * Maze::cEmpty. Row # of the layout array is set via the (mandatory)
	 * configuration option "maze-layout#" (where # is replaced with the row
	 * number). */
	char **m_maze_layout;

	/** The maximum possible reward in a single cycle. This is equivalent to
	 * the maximum value in Maze::m_maze_rewards. */
	int m_max_reward;

	/** The row occupied by the agent. */
	int m_row;

	/** The column occupied by the agent. */
	int m_col;

	/** Indicates whether the last action caused the agent to teleport. */
	bool m_teleported;

	/** Indicates whether the last action caused the agent to collide with a
	 * wall (Maze::cWall). */
	bool m_wall_collision;

	static const action_t aLeft;  /*!< Action: the agent moves left. */
	static const action_t aUp;    /*!< Action: the agent moves up. */
	static const action_t aRight; /*!< Action: the agent moves right. */
	static const action_t aDown;  /*!< Action: the agent moves down. */

	/** Uninformative observation used when Maze::m_obs_encoding is set to
	 * give uninformative observations. */
	static const percept_t oNull;

	/** Observation: A wall is present immediately to the left of the agent. */
	static const percept_t oLeftWall;

	/** Observation: A wall is present immediately above the agent. */
	static const percept_t oUpWall;

	/** Observation: A wall is present immediately to the right of the agent. */
	static const percept_t oRightWall;

	/** Observation: A wall is present immediately below the agent. */
	static const percept_t oDownWall;

	/** The type and encoding of observations provided to the agent. Default is
	 * uninformative observations. */
	enum {
		cUninformative, /*!< Unchanging observation. */
		cWalls,         /*!< Observe presence of adjacent walls. */
		cCoordinates    /*!< Observe coordinates of current square. */
	} m_obs_encoding;

	/** Represents an impassable square in the maze. */
	static const char cWall;

	/** Represents an empty square in the maze which can be teleported to. */
	static const char cTeleportTo;

	/** Represents an empty square in the maze which, upon entry, causes the
	 * agent to teleport to another square of type Maze::cTeleportTo. */
	static const char cTeleportFrom;

	/** Represents an empty square in the maze which cannot be teleported to or
	 * from. */
	static const char cEmpty;
};

#endif // __MAZE_HPP__

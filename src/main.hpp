#ifndef __MAIN_HPP__
#define __MAIN_HPP__

#include <fstream>
#include <map>
#include <string>
#include <vector>

/** A log of the agents interactions with the environment is written to this
 * stream in comma-separated-value format.
 */
extern std::ofstream logger;

/** The symbols that can be predicted. */
typedef bool symbol_t;

/** A list of symbols. */
typedef std::vector<symbol_t> symbol_list_t;

// TODO: make reward_t into long long?
/** Describes the reward accumulated by an agent. */
typedef double reward_t;

/** Describes an interaction (observation, reward, or action) between the agent
 * and the environment. */
typedef int interaction_t;

/** Describes a percept (observation or reward). */
typedef interaction_t percept_t;

/** Describes an agent action. */
typedef interaction_t action_t;

/** Describes the age of an agent. */
typedef long long age_t;

/** The program's keyword/value option pairs. */
typedef std::map<std::string, std::string> options_t;

#endif // __MAIN_HPP__

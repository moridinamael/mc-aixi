#include <cassert>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <sstream>

#include "agent.hpp"
#include "environment.hpp"
#include "main.hpp"
#include "search.hpp"
#include "util.hpp"

// Environments
#include "coinflip.hpp"
#include "kuhnpoker.hpp"
#include "maze.hpp"
#include "pacman.hpp"
#include "rock-paper-scissors.hpp"
#include "tictactoe.hpp"
#include "tiger.hpp"
#include "extendedtiger.hpp"


// Stream for logging
std::ofstream logger; // A compact comma-separated value log

/** The main agent/environment interaction loop. Each interaction cycle begins
 * with the agent receiving an observation and reward from the environment.
 * Subsequently, the agent selects an action and informs the environment. The
 * interactions that took place are logged to the ::logger and ::compactLogger
 * streams. When the cycle equals a power of two, a summary of the interactions
 * is printed to the standard output.
 * \param ai The agent.
 * \param env The environment.
 * \param options The configuration options. */
void mainLoop(Agent &ai, Environment &env, options_t &options) {

	// Apply random seed (Defaut: 0)
	srand(getOption<unsigned int>(options, "random-seed", 0));

	// Verbose output (Default: false)
	bool verbose = getOption<bool>(options, "verbose", false);

	// Determine exploration options (Default: don't explore, don't decay)
	bool explore = options.count("exploration") > 0;
	double explore_rate = getOption<double>(options, "exploration", 0.0);
	double explore_decay = getOption<double>(options, "explore-decay", 1.0);
    assert(0.0 <= explore_rate);
	assert(0.0 <= explore_decay && explore_decay <= 1.0);

	// Determine termination age (Default: don't terminate)
	bool terminate_check = options.count("terminate-age") > 0;
	age_t terminate_age = getOption<age_t>(options, "terminate-age", 0);
	assert(0 <= terminate_age);
	
	// Determine the cycle after which the agent stops learning (if ever)
	bool stop_learning = options.count("learning-period") > 0;
	int learning_period = getOption<int>(options, "learning-period", 0);
	assert(0 <= learning_period);

	// Agent/environment interaction loop
	for (int cycle = 1; !env.isFinished(); cycle++) {

		// Check for agent termination
		if (terminate_check && ai.age() > terminate_age) {
			break;
		}
		
		// Save the current clock cycle (to compute how long this cycle took)
		clock_t cycle_start = clock();

		// Get a percept from the environment
		percept_t observation = env.getObservation();
		percept_t reward = env.getReward();


		if (learning_period > 0 && cycle > learning_period)
			explore = false;
		
		// Update agent's environment model with the new percept
		ai.modelUpdate(observation, reward);

		// Determine best exploitive action, or explore
		action_t action;
		bool explored = false;

		if (explore && (rand01() < explore_rate)) { // Explore
			explored = true;
			action = ai.genRandomAction();
		}
		else { // Exploit
			action = ai.search();
		}

		// Send an action to the environment
		env.performAction(action);
		
		// Update agent's environment model with the chosen action
		ai.modelUpdate(action);
		
		// Calculate how long this cycle took
		double time = double(clock() - cycle_start) / double(CLOCKS_PER_SEC);

		// Log this turn
		logger << cycle << ", " << observation << ", " << reward << ", "
			<< action << ", " << explored << ", " << explore_rate << ", "
			<< ai.totalReward() << ", " << ai.averageReward() << ", "
			<< time << ", " << ai.modelSize() << std::endl;

		// Print to standard output when cycle == 2^n or on verbose option
		if (verbose || (cycle & (cycle - 1)) == 0) {
			std::cout << "cycle: " << cycle << std::endl;
			std::cout << "average reward: " << ai.averageReward() << std::endl;
			if (explore) {
				std::cout << "explore rate: " << explore_rate << std::endl;
			}
		}

		// Print environment state if verbose option is true
		if (verbose) {
  		    std::cout << env.print();
		}

		// Update exploration rate
		if (explore) explore_rate *= explore_decay;

	}

	// Print summary to standard output
	std::cout << std::endl << std::endl << "SUMMARY" << std::endl;
	std::cout << "agent age: " << ai.age() << std::endl;
	std::cout << "average reward: " << ai.averageReward() << std::endl;
}


/** Parse configuration options from a stream. Each line of the stream should
 * be a key/value pair of the form "key=value". Whitespace and anything
 * following a comment "#" character are ignored.
 * \param in The stream from which to read the configuration options.
 * \param options The map which will populated with the options.
 */
void processOptions(std::ifstream &in, options_t &options) {
	std::string line;
	size_t pos;

	// Loop through all the lines in the input stream, attempt to read a
	// configuration option on each line.
	for (int lineno = 1; in.good(); lineno++) {
		std::getline(in, line);
		
		if(line.size() == 0) {
			continue;
		}

		// Ignore # comments
		if ((pos = line.find('#')) != std::string::npos) {
			line = line.substr(0, pos);
		}

		// Remove whitespace
		while ((pos = line.find(" ")) != std::string::npos)
			line.erase(line.begin() + pos);
		while ((pos = line.find("\t")) != std::string::npos)
			line.erase(line.begin() + pos);
		while ((pos = line.find("\r")) != std::string::npos)
			line.erase(line.begin() + pos);

		// Split into key/value pair at the first '='
		pos = line.find('=');
		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);

		// Check that we have parsed a valid key/value pair. Warn on failure or
		// set the appropriate option on success.
		if (pos == std::string::npos) {
			std::cerr << "WARNING: processOptions skipping line " << lineno
			          << " (no '=')" << std::endl;
		}
		else if (key.size() == 0) {
			std::cerr << "WARNING: processOptions skipping line " << lineno
			          << " (no key)" << std::endl;
		}
		else if (value.size() == 0) {
			std::cerr << "WARNING: processOptions skipping line " << lineno
					  << " (no value)" << std::endl;
		}
		else {
			options[key] = value; // Success!
		}

	}
}

/** Entry point of the program. Sets up logging, default configuration values,
 * environment and agent before starting the agent/environment interaction cycle
 * by calling mainLoop(). In the case of invalid command line arguments, it
 * prints help information to the standard output and exits.
 */
int main(int argc, char *argv[]) {
	// Check valid command line arguments.
	if (argc < 2 || argc > 3) {
		std::cerr << "ERROR: Incorrect number of arguments" << std::endl
		    << "The first argument should indicate the location of the "
		    << "configuration file and the second (optional) argument should "
		    << "indicate the file to log to." << std::endl;
		return EXIT_FAILURE;
	}

	// Set up logging, print header
	logger.open(argv[2]);
	logger << "cycle, observation, reward, action, explored, "
	    << "explore_rate, total reward, average reward, time, model size"
	    << std::endl;


	// Stores configuration options
	options_t options;

	// Default configuration values
	options["ct-depth"] = "30";
	options["agent-horizon"] = "5";
	options["exploration"] = "0.0";     // do not explore
	options["explore-decay"] = "1.0"; // exploration rate does not decay
	options["mc-simulations"] = "300";

	// Read configuration options
	std::ifstream conf(argv[1]);
	if (!conf.is_open()) {
		std::cerr << "ERROR: Could not open file '" << argv[1]
		    << "' now exiting" << std::endl;
		return EXIT_FAILURE;
	}
	processOptions(conf, options);
	conf.close();

	// Set up the environment.
	Environment *env;
	std::string environment_name;
	getRequiredOption(options, "environment", environment_name);
	if (environment_name == "coin-flip") {
		env = new CoinFlip(options);
	} else if (environment_name == "extended-tiger") {
		env = new ExtendedTiger(options);
	} else if (environment_name == "kuhn-poker") {
		env = new KuhnPoker(options);
	} else if (environment_name == "maze") {
		env = new Maze(options);
	} else if (environment_name == "pacman") {
		env = new PacMan(options);
	} else if (environment_name == "rock-paper-scissors") {
		env = new RockPaperScissors(options);
	} else if (environment_name == "tictactoe") {
		env = new TicTacToe(options);
	} else if (environment_name == "tiger") {
 		env = new Tiger(options);
 	} else {
		std::cerr << "ERROR: unknown environment '" << environment_name << "'"
		    << std::endl;
		return EXIT_FAILURE;
	}

	// Copy environment-related configuration options to options map
	options["action-bits"] = toString(env->actionBits());
	options["observation-bits"] = toString(env->observationBits());
	options["percept-bits"] = toString(env->perceptBits());
	options["reward-bits"] = toString(env->rewardBits());
	options["max-action"] = toString(env->maxAction());
	options["max-observation"] = toString(env->maxObservation());
	options["max-reward"] = toString(env->maxReward());

	// Print options
	options_t::iterator it = options.begin();
	for( ; it != options.end(); it++) {
		std::cout << "OPTION: '" << it->first << "' = '" << it->second
		          << "'" << std::endl;
	}

	// Set up the agent
	Agent ai(options, *env);

	// Run the main agent/environment interaction loop
	mainLoop(ai, *env, options);

	logger.close();

	return EXIT_SUCCESS;
}

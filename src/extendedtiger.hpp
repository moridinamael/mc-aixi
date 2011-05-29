#ifndef __EXTENDED_TIGER_HPP__
#define __EXTENDED_TIGER_HPP__
#include "environment.hpp"
#include "main.hpp"

/** The environment is a more elaborate version of Tiger. There are two doors
 * and a stool. A tiger is hidden behind one door and a pot of gold is hidden
 * behind the other. The agent begins each round sitting on the stool where it
 * may either listen for the tiger (ExtendedTiger::aListen) or stand up
 * (ExtendedTiger::aStand). Listening for the tiger results in an observation
 * which correctly describes the tiger's whereabouts with probability
 * ExtendedTiger::m_listen_accuracy and a reward of ExtendedTiger::rListen.
 * Standing up result in an uninformative observation (ExtendedTiger::oNull) and
 * a reward of ExtendedTiger::rStand. Once the agent is standing, it may open
 * either the left or right door (ExtendedTiger::oLeft and
 * ExtendedTiger::oRight). Doing so results in an uninformative observation
 * (ExtendedTiger::oNull) and a reward based on what is behind the door
 * (ExtendedTiger::rGold or ExtendedTiger::rTiger). After opening a door the
 * agent is re-seated and the tiger and gold randomly re-allocated to a door
 * (ExtendedTiger::reset()). Attempting to open a door while seated, to listen
 * while standing, or to stand while already standing will result in an
 * uninformative observation (ExtendedTiger::oNull) and a reward of
 * ExtendedTiger::rInvalid.
 *
 * Domain characteristics:
 * - environment: "extended-tiger"
 * - maximum action: 3 (2 bits)
 * - maximum observation: 2 (2 bits)
 * - maximum reward: 130 (8 bits)
 *
 * Configuration options:
 * - tiger-listen-accuracy (optional): probability that listening (while seated)
 *   will correctly locate the door which hides the tiger. Must be a floating
 *   point value between 0.0 and 1.0 inclusive. Stored in
 *   ExtendedTiger::m_listen_accuracy. Default value is
 *   ExtendedTiger::cDefaultListenAccuracy. */
class ExtendedTiger: public Environment{
public:
	ExtendedTiger(options_t &options);

	virtual action_t maxAction() const { return 3; }
	virtual percept_t maxObservation() const { return 2; }
	virtual percept_t maxReward() const { return 130; }

	virtual void performAction(const action_t action);

	virtual std::string print() const;

private:
	/** Randomly place the tiger behind one door, the gold behind the other,
	 * and re-seat the agent. */
	void reset();

	/** True if the agent is sitting, false if the agent is standing. */
	bool m_sitting;

	/** Observation corresponding to the door containing the tiger. */
	percept_t m_tiger;

	/** Observation corresponding to the door containing the gold. */
	percept_t m_gold;

	/** Probability that listening (while sitting down) correctly identifies
	 * the door which hides the tiger. Default is
	 * ExtendedTiger::cDefaultListenAccuracy. */
	double m_listen_accuracy;

	/** Default value of ExtendedTiger::m_listen_accuracy. */
	static const double cDefaultListenAccuracy;

	/** Action: listen for tiger. */
	static const action_t aListen;

	/** Action: open left door. */
	static const action_t aLeft;

	/** Action: open right door. */
	static const action_t aRight;

	/** ACtion: stand up. */
	static const action_t aStand;

	/** Uninformative observation. Received when opening a door, standing up,
	 * or listening while standing. */
	static const percept_t oNull;

	/** Observation: hear tiger at left door. */
	static const percept_t oLeft;

	/** Observation: hear tiger at right door. */
	static const percept_t oRight;

	/** Reward: invalid action. */
	static const percept_t rInvalid;

	/** Reward: eaten by tiger. */
	static const percept_t rTiger;

	/** Reward: successfully stand up. */
	static const percept_t rStand;

	/** Reward: attempt to listen while sitting. */
	static const percept_t rListen;

	/** Reward: find gold. */
	static const percept_t rGold;
};

#endif // __EXTENDED_TIGER_HPP__

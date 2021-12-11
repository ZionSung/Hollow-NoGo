/**
 * Framework for NoGo and similar games (C++ 11)
 * agent.h: Define the behavior of variants of the player
 *
 * Author: Theory of Computer Games (TCG 2021)
 *         Computer Games and Intelligence (CGI) Lab, NYCU, Taiwan
 *         https://cgilab.nctu.edu.tw/
 */

#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include <fstream>

#include "mcts.h"

#define WITHOUT_MCTS 0
#define N_MCTS 1
#define T_MCTS 2
#define DEFAULT_SIMULATION_COUNT 200
#define MAX_SIMULATION_COUNT 8000

class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			meta[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string property(const std::string& key) const { return meta.at(key); }
	virtual void notify(const std::string& msg) { meta[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }
	virtual std::string name() const { return property("name"); }
	virtual std::string role() const { return property("role"); }

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> meta;
};

/**
 * base agent for agents with randomness
 */
class random_agent : public agent {
public:
	random_agent(const std::string& args = "") : agent(args) {
		if (meta.find("seed") != meta.end())
			engine.seed(int(meta["seed"]));
	}
	virtual ~random_agent() {}

protected:
	std::default_random_engine engine;
};

/**
 * random player for both side
 * put a legal piece randomly
 */
class player : public random_agent {
public:
	player(const std::string& args = "") : random_agent("name=random role=unknown " + args),
		space(board::size_x * board::size_y), who(board::empty) {
		/* +++++++++++++++++++++++++ new add start +++++++++++++++++++++++++ */
		//std::cout << "args are: " << args << std::endl;
		get_mcts_simulation_count(args);
		/* +++++++++++++++++++++++++ new add end +++++++++++++++++++++++++ */
		if (name().find_first_of("[]():; ") != std::string::npos)
			throw std::invalid_argument("invalid name: " + name());
		if (role() == "black") who = board::black;
		if (role() == "white") who = board::white;
		if (who == board::empty)
			throw std::invalid_argument("invalid role: " + role());

		for (size_t i = 0; i < space.size(); i++)
			space[i] = action::place(i, who);
	}

	virtual action take_action(const board& state) {
		 
		// test (force to use mcts)
		//mcts_type = T_MCTS;
		//simulation_count = 40;

		if(mcts_type == WITHOUT_MCTS){ // random
			std::shuffle(space.begin(), space.end(), engine);
			//std::cout << "==============================" << std::endl; 
			//std::cout << "     RANDOM    MOVE: " << who << "\n" << std::endl;
			//std::cout << state << std::endl;
			//std::cout << "==============================" << std::endl;
			//std::cin.get();
			for (const action::place& move : space) {
				board after = state;
				//std::cout << "random move: " << move << std::endl; 
				if (move.apply(after) == board::legal)
					return move;
			}
		}
		else{ // MCTS
			UCT uct(who);
			//std::cout << "==============================" << std::endl; 
			//std::cout << "    MCTS INITIAL STATE: " << who << "\n" << std::endl;
			//std::cout << state << std::endl;
			//std::cout << "==============================" << std::endl; 
			//std::cin.get();
			action move = uct.UCT_Search(simulation_count, state, space);
			board after = state;
			if (move.apply(after) == board::legal){
				return move;
			}
		}
		return action();
	}
	virtual void get_mcts_simulation_count(std::string args){

		std::size_t index = args.find("mcts");
		if(index != std::string::npos){
			args = args.substr(index+5); // +5 pass over "mcts"
			std::cout << args << std::endl;

			// Get type of MCTS
			if(args.at(0) == 'N'){
				mcts_type = N_MCTS;
				simulation_count = DEFAULT_SIMULATION_COUNT;
			}
			else{
				mcts_type = T_MCTS;
				simulation_count = DEFAULT_SIMULATION_COUNT;
			}

			args = args.substr(2); // skip "?=" to get value
			
			// Get simulation count
			index = args.find(" ");
			if(index != std::string::npos){
				simulation_count = std::stoi(args.substr(0, index));
				if(simulation_count > MAX_SIMULATION_COUNT){
					simulation_count = MAX_SIMULATION_COUNT;
				}
			}
		}
		else{
			simulation_count = 0;
			mcts_type = WITHOUT_MCTS; // without mcts
		}

		std::cout << "----------" << std::endl;
		std::cout << "MCTS Detail: " << std::endl;
		std::cout << "TYPE: " << mcts_type << std::endl;
		std::cout << "Simulation Count: " << simulation_count << std::endl;
		std::cout << "----------" << std::endl;
	}

private:
	std::vector<action::place> space;
	board::piece_type who;
	int mcts_type;
	int simulation_count;
};

/* original random agent
class player : public random_agent {
public:
	player(const std::string& args = "") : random_agent("name=random role=unknown " + args),
		space(board::size_x * board::size_y), who(board::empty) {
		if (name().find_first_of("[]():; ") != std::string::npos)
			throw std::invalid_argument("invalid name: " + name());
		if (role() == "black") who = board::black;
		if (role() == "white") who = board::white;
		if (who == board::empty)
			throw std::invalid_argument("invalid role: " + role());
		for (size_t i = 0; i < space.size(); i++)
			space[i] = action::place(i, who);
	}

	virtual action take_action(const board& state) {
		std::shuffle(space.begin(), space.end(), engine);
		for (const action::place& move : space) {
			board after = state;
			if (move.apply(after) == board::legal)
				return move;
		}
		return action();
	}

private:
	std::vector<action::place> space;
	board::piece_type who;
};
*/
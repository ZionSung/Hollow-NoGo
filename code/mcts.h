#pragma once
#include <iostream>
#include <algorithm>
#include <chrono>       // std::chrono::system_clock
#include <vector>
#include <math.h>
#include <float.h>
#include "board.h"
#include "action.h"
#include "agent.h"

#define RANDOM_PLAY 5

class MCTS{
public:

    struct Node
    {
        int id;
        // State s
        board state;
        // Action a
        action a;
        // Visit Count N
        int N;
        // Reward Q
        board::reward Q;

        // Parent node
        Node *parent;

        // Child nodes
        std::vector<Node *>child;

        // actions
        std::vector<action> actions;
    };

    Node *add_new_node(board state, Node *parent)
    {
        Node *temp = new Node;
        temp->state = state;
        temp->parent = parent;
        temp->N = 0;
        temp->Q = 0;
        temp->id = id;
        id++;
        
        return temp;
    }

    bool nonterminal(board state, std::vector<action::place> space){
        //std::default_random_engine engine;
        //std::shuffle(space.begin(), space.end(), engine);
        for (const action::place& move : space) {
			board after = state; 
			if (move.apply(after) == board::legal){
                ////std::cout << "move is " << move << std::endl;
				return true;
            }
		}
        ////std::cout << "return false -- terminal" << std::endl;
        return false;
    }

    void update_actions(Node* node, std::vector<action::place> space){
        for (const action::place& move : space) {
			board after = node->state; 
			if (move.apply(after) == board::legal){
                node->actions.push_back(move);
            }
		}
        ////std::cout << "Node: " << node->id << " all posible actions number: " << node->actions.size() << std::endl;
        ////std::cout << "-- pause -- " << std::endl;
        //std::cin.get();
    }

    bool not_fully_expanded(Node* node){
        if(node->actions.size() == 0){
            return false;
        }
        else{
            return true; 
        }
    }

    action choose_action(Node* node){
        action move;
        if(node->actions.size() == 0){
            //std::cout << "Error: there's no more action can choose." << std::endl;
        }
        move = node->actions.back();
        node->actions.pop_back();
        return move;
    }

    action random_choose_action(board state, std::vector<action::place>& space){
        ////std::cout << "+++++ choose action +++++" << std::endl;
        
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(space.begin(), space.end(), std::default_random_engine(seed));
        //std::default_random_engine engine;
        //std::shuffle(space.begin(), space.end(), engine);
        for (const action::place& move : space) {
            ////std::cout << "+++++ choose action 1 +++++" << std::endl;
            ////std::cout << state << std::endl;
			board after = state; 
			if (move.apply(after) == board::legal){
                ////std::cout << "+++++ choose action 2 +++++" << std::endl;
                ////std::cout << after << std::endl;
				return move;
            }
		}
        //std::cout << "Warning: choose action in UCT" << std::endl;
        return action();
    }

private:
    int id = 0;
};

class UCT: MCTS{
public:
    
    //std::vector<const action::place&> fully_expanded_nodes; 
    UCT(board::piece_type who) : sim_who(who), true_player(who){} // who is playing now for simuation

    action UCT_Search(int simulation_count, board current_state, std::vector<action::place> space){
        // create root node V0 with state S0
        if(!nonterminal(current_state, space)) return action(); // GAME OVER
        Node *v0 = add_new_node(current_state, nullptr);
        update_actions(v0, space);
        v0->id = 0; // root id
        ////std::cout << "++++++ state v0 ++++++\n\n" << current_state << std::endl; 
        for(int i = 0; i < simulation_count; i++){
            ////std::cout << "Check space[0] is: " << space[0] << std::endl;
            Node *v = Tree_Policy(v0, space);
            //v->id = i+1;
            ////std::cout << "++++++ state vl ++++++\n" << v->state << std::endl;
            board::reward reward = Default_Policy(v->state, space);
            Backup_Negamax(v, reward);
            //std::cout << "-- simulation " << i << " finish --" << std::endl;
            //std::cin.get();
        }
        return Best_Child(v0, Cp)->a;
    }

    void Backup_Negamax(Node* v, board::reward reward){
        Node *temp = new Node;
        temp = v;
        while(temp->id != 0){
            //std::cout << "current id: " << temp->id << std::endl;
            //std::cout << "Old N: " << temp->N;
            temp->N += 1;
            //std::cout << " -> New N: " << temp->N << std::endl;
            //std::cout << "Old Q: " << temp->Q;
            temp->Q += reward;
            //std::cout << " -> New Q: " << temp->Q << std::endl;

            //reward = -reward;

            //std::cout << "parent id: " << temp->parent->id << std::endl;
            temp = temp->parent;
            //std::cout << "---- backup iter ---- " << std::endl;
            //std::cin.get();
        }
        // parent
        //std::cout << "root: " << temp->id << std::endl;
        //std::cout << "Old N: " << temp->N;
        temp->N += 1;
        //std::cout << " -> New N: " << temp->N << std::endl;
        //std::cout << "Old Q: " << temp->Q;
        temp->Q += reward;
        //std::cout << " -> New Q: " << temp->Q << std::endl;
        //std::cout << "---- backup end ---- " << std::endl;
    }

    Node *Tree_Policy(Node *v, std::vector<action::place> space){
        while(nonterminal(v->state, space)){
            ////std::cout << "=============================================" << std::endl;
            ////std::cout << "================ TREE POLICY ================" << std::endl;
            ////std::cout << "=============================================" << std::endl;
            ////std::cout << v->state << std::endl;
            if(not_fully_expanded(v)){
                return Expand(v, space);
            }
            else{
                //std::cout << "======================" << std::endl;
                //std::cout << "       SELECT      " << std::endl;
                //std::cout << "======================" << std::endl;
                v = Best_Child(v, Cp);

                // Change to another player
                ////std::cout << "In choosing best child Check space[0] is: " << space[0] << std::endl;
                space = change_to_another(space);
                ////std::cout << "In choosing best child Check space[0] is: " << space[0] << std::endl;

                //std::cout << "Best Child is " << v->id << std::endl;
                //std::cout << v->state << std::endl;
                //update_actions(v, space);
            }
        }
        ////std::cout << "Break nonterminal while loop." << std::endl;
        return v;
    }

    Node *Expand(Node *parent, std::vector<action::place> space){
        board after = parent->state;
        action move = choose_action(parent);
        move.apply(after);
        //std::cout << "+++++++++ Expand Node State +++++++++" << std::endl;
        ////std::cout << after << std::endl; 
        Node *child = add_new_node(after, parent);
        space = change_to_another(space);
        update_actions(child, space);
        //std::cout << child->state << std::endl;
        child->a = move;
        (parent->child).push_back(child);
        return child;
    }

    Node *Best_Child(Node *v, double c){
        double highest_reward = -DBL_MAX;
        Node* best_child = nullptr; 
        //double parent_N = v->N; // Cp != 0 need it
        for(Node* child : v->child){
            double Q = child->Q;
            double N = child->N;
            //std::cout << "paremt ID: " << v->id << std::endl;
            //std::cout << "parent_N:" << parent_N << std::endl;
            //std::cout << "N: " << N << std::endl;
            //std::cout << "Q: " << Q  << std::endl;
            //std::cout << "c: " << c << std::endl;
            //double q_n = Q/N;
            ////std::cout << "Q/N: " << q_n << std::endl;
            //double in_sqrt = sqrt( (2*log(parent_N)) / N);
            ////std::cout << "In sqrt: " << in_sqrt << std::endl;

            //double reward = (Q/N) + (c * (sqrt( (2*log(parent_N)) / N)));
            double reward = Q/N;

            //std::cout << "Reward: " << reward << std::endl;
            //double reward = (Q/N);
            //std::cout << "ID: " << child->id << " Reward: " << reward << std::endl;
            //std::cout << "-- Best Child iter pause -- " << std::endl;
            //std::cin.get();
            if(reward > highest_reward){
                highest_reward = reward;
                best_child = child;
            }
        }
        ////std::cout << "Inner best child ID: ";
        ////std::cout << best_child->id << std::endl;
        return best_child;
    }

    board::reward RandomPlay(board s, std::vector<action::place> space){
        //std::cout << "\n+++++++++++++++++++++" << std::endl;
        //std::cout << "++   simulations   ++" << std::endl;
        board::reward reward = 0;
        ////std::cout << "Check!" << std::endl;
        /*
         * Debug:
         *   Maybe we need to change to opposer.
         *   check play is black or white. 
         */
        board::piece_type who = sim_who;
        //std::cout << "who: " << who << std::endl;
        //std::cout << "sim_who: " << sim_who << std::endl;
        space = who_space_sync(sim_who, space);
        ////std::cout << "sim_who: " << sim_who << std::endl;

        while(nonterminal(s, space)){
            
            action move = random_choose_action(s, space);
            ////std::cout << "move is " << move << std::endl;
            move.apply(s);
            //reward = move.apply(s);
            ////std::cout << "reward = " << reward << std::endl;
            ////std::cout << "++++++ simulations who:" << sim_who << " ++++++\n" << s << std::endl;
            ////std::cout << "-- pause -- " << std::endl;
            //std::cin.get();

            space = change_to_another(space);
            ////std::cout << "loop sim_who: " << sim_who << std::endl;
        }
        ////std::cout << "last sim_who: " << sim_who << std::endl;
        if(sim_who == true_player){
            //std::cout << "You Loose..." << std::endl;
            reward = 0; // you loose
        }
        else{
            //std::cout << "You WIN!" << std::endl;
            reward = 1; // you win
        }

        sim_who = who; // recover
        ////std::cout << s << "++ simulations end ++" << std::endl;
        //std::cout << "++ simulations end ++" << std::endl;
        //std::cout << "++*****************++\n" << std::endl;

        return reward;
    }

    board::reward Default_Policy(board s, std::vector<action::place> space){
        board::reward reward = 0;
        for(int i = 0; i < RANDOM_PLAY; i++){
            reward += RandomPlay(s, space);
        }
        return reward;
    }

    std::vector<action::place>& who_space_sync(board::piece_type who,  std::vector<action::place>& space){

        if(who == board::white){
            for (size_t i = 0; i < space.size(); i++){
                space[i] = action::place(i, board::white);
            }
            sim_who = board::white;
        }
        else{
            for (size_t i = 0; i < space.size(); i++){
                space[i] = action::place(i, board::black);
            }
            sim_who = board::black;
        }
        return space;
    }

    std::vector<action::place>& change_to_another(std::vector<action::place>& space){

        if(space[0].color() == board::black){
            for (size_t i = 0; i < space.size(); i++){
                space[i] = action::place(i, board::white);
            }
            sim_who = board::white;
        }
        else{
            for (size_t i = 0; i < space.size(); i++){
                space[i] = action::place(i, board::black);
            }
            sim_who = board::black;
        }
        return space;
    }

private:
    board::piece_type sim_who;
    board::piece_type true_player;
    //std::vector<action> actions;
    double Cp = 0;//1/(sqrt(2));
};
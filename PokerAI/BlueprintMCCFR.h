#pragma once
#include <iostream>
#include <sys/time.h>
#include <random>
#include "tree/Node.h"
#include "poker/State.h"
#include "tree/Bulid_Tree.h"
#include "tree/Save_load.h"
#include "tree/Visualize_Tree.h"
#include "tree/Exploitability.h"
using namespace std;

double blueprint_cfr(strategy_node* cnode[], Pokerstate& state, int pi, double w) { // mccfr
	int ph = state.player_i_index;
	assert(cnode[0]->action_len == cnode[1]->action_len);
	if (state.is_terminal()) {
		return state.payout(pi);
	}
	else if (ph == pi) {
		double sigma[12];
		double vo = 0;
		calculate_strategy(cnode[ph]->regret, cnode[ph]->action_len, sigma);
		int len = cnode[0]->action_len;
		double voa[12] = { 0 };
		for (int i = 0; i < len; i++) {
			Pokerstate st2 = state;
			bool is_chance = st2.apply_action(cnode[ph]->actionstr[i]);
			strategy_node* cnode2[2];
			cnode2[0] = cnode[0]->actions + i;//cnode[0]->findnode(cnode[0]->actionstr[i]);
			cnode2[1] = cnode[1]->actions + i;//cnode[1]->findnode(cnode[1]->actionstr[i]);
			if (is_chance) {
				cnode2[0] = (cnode2[0]->actions + st2.table.players[0].clusters[st2.betting_stage]);
				cnode2[1] = (cnode2[1]->actions + st2.table.players[1].clusters[st2.betting_stage]);
			}
			voa[i] = blueprint_cfr(cnode2, st2, pi, w) * w;
			vo += sigma[i] * voa[i];
		}
		for (int i = 0; i < len; i++) {
			cnode[ph]->regret[i] += voa[i] - vo;
			assert(cnode[ph]->regret[i] < 200000000);
			if (cnode[ph]->regret[i] < -210000000)
				cnode[ph]->regret[i] = -210000000;
		}
		assert(w > 0);
		return vo / w;
	}
	else {
		double sigma[12];
		double vo = 0;
		calculate_strategy(cnode[ph]->regret, cnode[ph]->action_len, sigma);
		int len = cnode[0]->action_len;
		double voa[12] = { 0 };
		for (int i = 0; i < len; i++) {
			if (sigma[i] > 0) {
				Pokerstate st2 = state;
				bool is_chance = st2.apply_action(cnode[ph]->actionstr[i]);
				strategy_node* cnode2[2];
				cnode2[0] = cnode[0]->actions + i;//cnode[0]->findnode(cnode[0]->actionstr[i]);
				cnode2[1] = cnode[1]->actions + i;//cnode[1]->findnode(cnode[1]->actionstr[i]);
				if (is_chance) {
					cnode2[0] = (cnode2[0]->actions + st2.table.players[0].clusters[st2.betting_stage]);
					cnode2[1] = (cnode2[1]->actions + st2.table.players[1].clusters[st2.betting_stage]);
				}
				voa[i] = blueprint_cfr(cnode2, st2, pi, w * sigma[i]);
				vo += sigma[i] * voa[i];
			}
		}
		return vo;
	}
}
double blueprint_cfrp(strategy_node* cnode[], Pokerstate& state, int pi, int c, double w) { // cfr prune
	int ph = state.player_i_index;
	assert(cnode[0]->action_len == cnode[1]->action_len);
	if (state.is_terminal()) {
		return state.payout(pi);
	}
	else if (ph == pi) {
		double sigma[12];
		double vo = 0;
		calculate_strategy(cnode[ph]->regret, cnode[ph]->action_len, sigma);
		int len = cnode[0]->action_len;
		bool explored[15] = { false };
		double voa[15] = { 0 };
		for (int i = 0; i < len; i++) {
			if (cnode[ph]->regret[i] > c) {
				Pokerstate st2 = state;
				bool is_chance = st2.apply_action(cnode[ph]->actionstr[i]);
				strategy_node* cnode2[2];
				cnode2[0] = cnode[0]->actions + i;//cnode[0]->findnode(cnode[0]->actionstr[i]);
				cnode2[1] = cnode[1]->actions + i;//cnode[1]->findnode(cnode[1]->actionstr[i]);
				if (is_chance) {
					cnode2[0] = (cnode2[0]->actions + st2.table.players[0].clusters[st2.betting_stage]);
					cnode2[1] = (cnode2[1]->actions + st2.table.players[1].clusters[st2.betting_stage]);
				}
				if (st2.betting_stage < 2)
					voa[i] = blueprint_cfrp(cnode2, st2, pi, c, w) * w;
				else
					voa[i] = blueprint_cfr(cnode2, st2, pi, w) * w;
				explored[i] = true;
				vo += sigma[i] * voa[i];
			}
			else
				explored[i] = false;
		}
		for (int i = 0; i < len; i++)
			if (explored[i]) {
				cnode[ph]->regret[i] += voa[i] - vo;
				assert(cnode[ph]->regret[i] < 200000000);
				if (cnode[ph]->regret[i] < -210000000)
					cnode[ph]->regret[i] = -210000000;
			}
		assert(w > 0);
		return vo / w;
	}
	else {
		double sigma[12];
		double vo = 0;
		calculate_strategy(cnode[ph]->regret, cnode[ph]->action_len, sigma);
		int len = cnode[0]->action_len;
		double voa[12] = { 0 };
		for (int i = 0; i < len; i++) {
			if (sigma[i] > 0) {
				Pokerstate st2 = state;
				bool is_chance = st2.apply_action(cnode[ph]->actionstr[i]);
				strategy_node* cnode2[2];
				cnode2[0] = cnode[0]->actions + i;//cnode[0]->findnode(cnode[0]->actionstr[i]);
				cnode2[1] = cnode[1]->actions + i;//cnode[1]->findnode(cnode[1]->actionstr[i]);
				if (is_chance) {
					cnode2[0] = (cnode2[0]->actions + st2.table.players[0].clusters[st2.betting_stage]);
					cnode2[1] = (cnode2[1]->actions + st2.table.players[1].clusters[st2.betting_stage]);
				}
				voa[i] = blueprint_cfr(cnode2, st2, pi, w * sigma[i]);
				vo += sigma[i] * voa[i];
			}
		}
		return vo;
	}
}

void dfs_discount(strategy_node* treenode,double d, bool firstin) {
	if (treenode->action_len == 0)
		return;
	if (treenode->action_len > 100) {
		if (firstin) {
			dfs_discount(treenode->actions, d, true);
			for (int i = 1; i < treenode->action_len; i++)
				dfs_discount(treenode->actions + i, d, false);
		}
		return;
	}
	else {
		double sigma[15];
		calculate_strategy(treenode->regret, treenode->action_len, sigma);
		for (int i = 0; i < treenode->action_len; i++) {
			if (sigma[i] > 0)
				treenode->averegret[i] += sigma[i];
			treenode->regret[i] *= d;
			//treenode->averegret[i] *= d;
			dfs_discount(treenode->actions + i, d, firstin);
		}
	}
}

void update_strategy(strategy_node* treenode, bool firstin) {
	if (treenode->action_len == 0)
		return;
	if (treenode->action_len > 100) {
		if (firstin) {
			update_strategy(treenode->actions, true);
			for (int i = 1; i < treenode->action_len; i++)
				update_strategy(treenode->actions + i, false);
		}
		return;
	}
	else{
		double sigma[15];
		calculate_strategy(treenode->regret, treenode->action_len, sigma);
		for (int i = 0; i < treenode->action_len; i++) {
			if (sigma[i] > 0) 
				treenode->averegret[i] += sigma[i];
			update_strategy(treenode->actions + i, firstin);
		}
	}
}

const ll strategy_interval = 100000, discount_interval = 1000000, n_iterations = 2000000000, lcfr_threshold = 400000000;
const int prune_threshold = 100000000, c = -200000000, n_players = 2, print_iteration = 10, dump_iteration = 100000000, update_threshold = 1000000;

void Singleiter() {
	struct timeval start, end;
	Player players[] = { Player(20000),Player(20000) };
	PokerTable table(2, players);
	strategy_node* pref[2];
	Pokerstate state(table);
	state.reset_game();
	strategy_node* root = new strategy_node();
	bulid_preflop(root, state); 
	check_subgame(root, state);
	gettimeofday(&start, NULL);

	mt19937_64 _rng_gen(rand());
 	for (ll t = 1; t <= n_iterations; t++) {
		if (t % 10000 == 0) {
			_rng_gen.seed(rand());
			gettimeofday(&end, NULL);
			cout << "10000 time:" << ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)) / 1000000.0 << endl;
			cout << "iter:" << t << endl;
			gettimeofday(&start, NULL);
		}
		state.reset_game_single();
		for (int i = 0; i < n_players; i++) {
			state.reset_game();

			pref[0] = (root->actions + state.table.players[0].clusters[0]);
			pref[1] = (root->actions + state.table.players[1].clusters[0]);

			if (t > prune_threshold) {
				int dr = rand() % 100;
				if (dr < 5)
					blueprint_cfr(pref, state, i, 1);
				else
					blueprint_cfrp(pref, state, i, c, 1);
			}
			else
				blueprint_cfr(pref, state, i, 1);
		}
		if (t % strategy_interval == 0)
			update_strategy(root, true);
		if (t < lcfr_threshold && t % discount_interval == 0) {
			double d = ((double)t / discount_interval) / (((double)t / discount_interval) + 1);
			dfs_discount(root, d, true);
		}
		if (t % dump_iteration == 0) {
			dump(root, "blueprint_strategy.dat");
		}

	}
}

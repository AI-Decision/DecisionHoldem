#pragma once
#include "BlueprintMCCFR.h"
#include "util/ThreadPool.h"
#include "tree/Exploitability.h"
using namespace std;

const int threadnum = 100;
const int multistrategy_interval = 5000, multin_iterations = 100000000, multilcfr_threshold = 4000000, multidiscount_interval = 10000;
const int multiprune_threshold = 100000, multic = -200000000, multin_players = 2, multidump_iteration = 100000, multiupdate_threshold = 1000;
const int each_thread_iters = 2000;
strategy_node* root = new strategy_node();

void each_thread(int t) {

	struct timeval start, end;
	gettimeofday(&start, NULL);
	mt19937_64 _rng_gen(rand());
	Player players[] = { Player(20000),Player(20000) };
	PokerTable table(2, players);
	strategy_node* pref[2];

	Pokerstate state(table);
	for (int k = 1; k <= each_thread_iters; k++) {
		state.reset_game_single();
		for (int i = 0; i < n_players; i++) {
			state.reset_game();
			pref[0] = root->actions + state.table.players[0].clusters[0];
			pref[1] = root->actions + state.table.players[1].clusters[0];
			if (t > multiprune_threshold) {
				int dr = rand() % 100;
				if (dr < 5)
					blueprint_cfr(pref, state, i, 1);
				else
					blueprint_cfrp(pref, state, i, multic, 1);
			}
			else
				blueprint_cfr(pref, state, i, 1);
		}
	}
	delete state.table.deck.cards;
	for (int i = 0; i < state.table.playerlen; i++)
		delete state.table.players[i].clusters;
	gettimeofday(&end, NULL);
	cout << "iter :" << t << ",each thread time:" << ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)) / 1000000.0 << endl;

}
void multiprocess_blueprint() {			//program exit

	{
		Player players[] = { Player(20000),Player(20000) };
		PokerTable table(2, players);
		Pokerstate state(table);
		state.reset_game();
		bulid_preflop(root, state);

		state.reset_game();
		check_subgame(root, state);
		//state.reset_game();
		//cout << getcfv_whole_holdem(root, state, 0) << endl;
		//state.reset_game();
		//cout << getcfv_whole_holdem(root, state, 1) << endl;
		delete[] players[0].clusters;
		delete[] players[1].clusters;
		delete[] table.deck.cards;
	}
	
	std::condition_variable dumpwait;
	mutex mtx;
	threadpool pool(threadnum, &dumpwait);
	srand(time(0));
	for (int t = 1; t <= multin_iterations; t++) {
		std::unique_lock<mutex> lck(mtx);
		pool.commit(each_thread, t);
		if (t % multistrategy_interval == 0) {
			dumpwait.wait(lck);
			while (pool.acttaskNum.load() != 0)
				sleep(1);
			cout << "iter :" << t << endl;
			if (t <= multilcfr_threshold && t % multidiscount_interval == 0) {
				double d = ((double)t / multidiscount_interval) / (((double)t / multidiscount_interval) + 1);
				dfs_discount(root, d, true);
			}
			else
				update_strategy(root, true);
			if (t % multidump_iteration == 0) {
				dump(root, "cluster/blueprint_strategy.dat");
			}
		}
	}
}

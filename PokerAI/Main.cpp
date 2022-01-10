#include <random>
#include "Multi_Blureprint.h"

int main(int argc, char **argv) {			
	assert(argc == 1);
	if (argv[0] == 0)
		multiprocess_blueprint();
	else
	{
		Player players[] = { Player(20000),Player(20000) };
		PokerTable table(2, players);
		Pokerstate state(table);
		state.reset_game();
		strategy_node* root = new strategy_node();
		load(root, "cluster/blueprint_strategy.dat");
		state.reset_game(); 
		check_subgame(root, state);
		state.reset_game();
		cout << getcfv_whole_holdem(root, state, 0) << endl;
		state.reset_game();
		cout << getcfv_whole_holdem(root, state, 1) << endl;
	}
}

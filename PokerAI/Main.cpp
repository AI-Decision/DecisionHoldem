################################################################################
#
#   Copyright 2022 The DecisionHoldem Authors，namely，Qibin Zhou，
#   Dongdong Bai，Junge Zhang and Kaiqi Huang. All Rights Reserved.
#
#   Licensed under the GNU AFFERO GENERAL PUBLIC LICENSE
#                 Version 3, 19 November 2007
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU Affero General Public License for more details.
#
#   You should have received a copy of the GNU Affero General Public License 
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
################################################################################
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

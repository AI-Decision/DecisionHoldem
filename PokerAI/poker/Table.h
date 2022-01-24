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
#pragma once
#include "Player.h"
#include "Card.h"
#include "Deck.h"
class PokerTable {
public:
	Player players[2];
	unsigned char playerlen;
	int total_pot; 
	Deck deck;
	PokerTable() {}
	PokerTable(int _playerlen, Player _players[]) {
		this->playerlen = _playerlen;
		total_pot = 0;
		for (int i = 0; i < playerlen; i++)
			players[i] = _players[i];
	}
	void reset() {
		total_pot = 0;
		//deck.reset();
		for (int i = 0; i < playerlen; i++)
			players[i].reset();
	}
	int total() {
		int ans = 0;
		for (int i = 0; i < playerlen; i++)
			ans += players[i].n_bet_chips();
		return ans;
	}
	inline void Add_pot(int addchips) {
		total_pot += addchips;
	}
	void _assign_blinds(int small_blind, int big_blind) {
		players[0].add_to_pot(small_blind);
		players[1].add_to_pot(big_blind);
		total_pot = small_blind + big_blind;
	}
};
class SearchTable {
public:
	SearchPlayer players[2];
	unsigned char playerlen;//simulatechance:chance节点模拟十次计算平均utility，cur_simulatechance是当前到第几个了
	unsigned short clusters[2][4];
	int total_pot;
	Deck deck;
	SearchTable(int _playerlen = 2) {
		this->playerlen = _playerlen;
		total_pot = 0;
	}
	void reset() {
		total_pot = 0;
		for (int i = 0; i < playerlen; i++)
			players[i].reset();
	}
	int total() {
		int ans = 0;
		for (int i = 0; i < playerlen; i++)
			ans += players[i].n_bet_chips();
		return ans;
	}
	inline void Add_pot(int addchips) {
		total_pot += addchips;
	}
	void _assign_blinds(int small_blind, int big_blind) {
		players[0].add_to_pot(small_blind);
		players[1].add_to_pot(big_blind);
		total_pot = small_blind + big_blind;
	}
};

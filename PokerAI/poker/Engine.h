#pragma once
#include <sys/time.h>
#include <assert.h>
#include <fstream>
#include "Table.h"
#include <vector>
#include<algorithm>
#include <set>
#include <string.h>
using namespace std;
#define ll unsigned long long

struct river_community {
	unsigned* keys;
	unsigned short* values;
};
struct turn_community {
	unsigned* keys;
	unsigned* values;
};
struct flop_community {
	unsigned* keys;
	unsigned* values;
};

static const ll length_river = 2809475760;
const int river_community_total = 2118760;
const int rivern_rank_divde = 1981;
river_community river_cluster[2652];
river_community river_cluster2[2652];

static const int length_turn = 305377800;
const int turn_community_total = 230300;
const int turn_rank_divde = 86966 + 2; //1980*46=91080; //86966 / 200 = 434.83 ,218
turn_community turn_cluster[2652];

static const int length_flop = 25989600;
const int flop_community_total = 19600;
const int flop_rank_divde = 1903365 + 3; //91080 * 47 / 2 = 2140380;
flop_community flop_cluster[2652];

static const int seven_length = 133784560;
ll seven_keys[seven_length];
unsigned short seven_strengths[seven_length];


static const int length_preflop = 1326;
static const int prim_preflop = 2652; //50*51=2651
static unsigned char preflop_cluster[prim_preflop];

int* preflop_allin[2652];
const int preflop_allin_divde = 1712304;
class Engine {
public:
	Engine() {
		load();
	}

	void load() {
		ifstream in("cluster/sevencards_strength.bin", ios::in | ios::binary);
		if (!in) {
			cout << "hand_value file is not exist";
			throw exception();
		}
		in.read((char*)seven_keys, sizeof seven_keys);
		in.read((char*)seven_strengths, sizeof seven_strengths);
		in.close();
		unsigned preflop_cluster_key[length_preflop] = { 0 };
		unsigned preflop_cluster_value[length_preflop] = { 0 };
		for (int i = 0; i < prim_preflop; i++) preflop_cluster[i] = 169;
		ifstream in3("cluster/preflop_hand_cluster.bin", ios::in | ios::binary);
		if (!in3) {
			cout << "hand_value file is not exist";
			throw exception();
		}
		in3.read((char*)preflop_cluster_key, sizeof preflop_cluster_key);
		in3.read((char*)preflop_cluster_value, sizeof preflop_cluster_value);
		for (int i = 0; i < length_preflop; i++)
			preflop_cluster[preflop_cluster_key[i]] = preflop_cluster_value[i];
		in3.close();

		for (int i = 0; i < 51; i++)
			for (int j = i + 1; j < 52; j++) {
				turn_cluster[i * 52 + j].keys = new unsigned[turn_community_total];
				turn_cluster[i * 52 + j].values = new unsigned[turn_community_total];
			}
		ifstream in4("cluster/turn_hand_cluster.bin", ios::in | ios::binary);
		for (int i = 0; i < 51; i++)
			for (int j = i + 1; j < 52; j++) {
				in4.read((char*)turn_cluster[i * 52 + j].keys, sizeof(unsigned) * 230300);
				in4.read((char*)turn_cluster[i * 52 + j].values, sizeof(unsigned) * 230300);
			}
		in4.close();

		for (int i = 0; i < 51; i++)
			for (int j = i + 1; j < 52; j++) {
				flop_cluster[i * 52 + j].keys = new unsigned[flop_community_total];
				flop_cluster[i * 52 + j].values = new unsigned[flop_community_total];
			}
		ifstream in5("cluster/flop_hand_cluster.bin", ios::in | ios::binary);
		for (int i = 0; i < 51; i++)
			for (int j = i + 1; j < 52; j++) {
				in5.read((char*)flop_cluster[i * 52 + j].keys, sizeof(unsigned) * 19600);
				in5.read((char*)flop_cluster[i * 52 + j].values, sizeof(unsigned) * 19600);
			}
		in5.close();
		for (int i = 0; i < 51; i++)
			for (int j = i + 1; j < 52; j++) {
				river_cluster[i * 52 + j].keys = new unsigned[river_community_total];
				river_cluster[i * 52 + j].values = new unsigned short[river_community_total];
			}
		ifstream in6("cluster/river_hand_cluster.bin", ios::in | ios::binary);
		for (int i = 0; i < 51; i++)
			for (int j = i + 1; j < 52; j++) {
				in6.read((char*)river_cluster[i * 52 + j].keys, sizeof(unsigned) * river_community_total);
				in6.read((char*)river_cluster[i * 52 + j].values, sizeof(unsigned short) * river_community_total);
			}
		in6.close();
		for (int i = 0; i < 51; i++)
			for (int j = i + 1; j < 52; j++) {
				preflop_allin[i * 52 + j] = new int[2652];
			}
		ifstream in8("cluster/preflopallin1326.1225.bin", ios::in | ios::binary);
		for (int i = 0; i < 51; i++)
			for (int j = i + 1; j < 52; j++) 
				in8.read((char*)preflop_allin[i * 52 + j], sizeof(int) * 2652);
		in8.close();

	}
	

	int get_preflop_cluster(int c1, int c2) { //get preflop card 1326 to 169
		unsigned cnt = c1 * 52 + c2;
		assert(preflop_cluster[cnt] >= 0 && preflop_cluster[cnt] < 169);
		return preflop_cluster[cnt];
	}
	int get_preflop_cluster(unsigned char  hand[]) { //get preflop card 1326 to 169
		unsigned cnt;
		if (hand[0] <= hand[1])
			cnt = hand[0] * 52 + hand[1];
		else
			cnt = hand[1] * 52 + hand[0];
		if (preflop_cluster[cnt] < 0 || preflop_cluster[cnt] >= 169) {
			cout << "preflop_cluster[cnt] <= 0 || preflop_cluster[cnt] > 169:" << preflop_cluster[cnt] << endl;
			throw exception();
		}
		return preflop_cluster[cnt];
	}
	void sortp(unsigned char evalcards[], int len) {
		for (int i = 0; i < len - 1; i++) {
			for (int j = 0; j < len - 1 - i; j++) {
				if (evalcards[j] > evalcards[j + 1]) {
					unsigned temp = evalcards[j];
					evalcards[j] = evalcards[j + 1];
					evalcards[j + 1] = temp;
				}
			}
		}
	}
	unsigned get_flop_cluster(unsigned a1, unsigned a2, unsigned char com[]) { //get flop card to 200
		unsigned char comm[] = { com[0],com[1],com[2] };
		sortp(comm, 3);
		unsigned rank = find_flop(a1 * 52 + a2, comm[0] * 2704 + comm[1] * 52 + comm[2]);
		//if (rank) rank += 9517; //1903365 / 200 = 9516.825
		//rank = (int)((rank / (double)flop_rank_divde) * 50000);
		assert(rank >= 0 && rank < 50000);
		return rank;
	}
	unsigned get_flop_cluster(unsigned char hand[], unsigned char  com[]) { //get flop card to 200
		unsigned a1, a2;
		if (hand[0] > hand[1]) {
			a1 = hand[1];
			a2 = hand[0];
		}
		else {
			a1 = hand[0];
			a2 = hand[1];
		}
		unsigned char comm[] = { com[0],com[1],com[2] };
		sortp(comm, 3);
		unsigned rank = find_flop(a1 * 52 + a2, comm[0] * 2704 + comm[1] * 52 + comm[2]);
		//if (rank) rank += 9517; //1903365 / 200 = 9516.825
		//rank = (int)((rank / (double)flop_rank_divde) * 50000);
		assert(rank >= 0 && rank < 50000);
		return rank;
	}
	unsigned get_turn_cluster(unsigned a1, unsigned a2, unsigned char com[]) { //get turn card to 200
		unsigned char comm[] = { com[0],com[1],com[2],com[3] };
		sortp(comm, 4);
		unsigned rank = find_turn(a1 * 52 + a2, comm[0] * 140608 + comm[1] * 2704 + comm[2] * 52 + comm[3]);
		//if (rank) rank += 435; //86966 / 200 = 434.83
		//rank = (int)((rank / (double)turn_rank_divde) * 5000);
		assert(rank >= 0 && rank < 5000);
		return rank;
	}
	unsigned get_turn_cluster(unsigned char  hand[], unsigned char  com[]) { //get turn card to 200
		unsigned a1, a2;
		if (hand[0] > hand[1]) {
			a1 = hand[1];
			a2 = hand[0];
		}
		else {
			a1 = hand[0];
			a2 = hand[1];
		}
		unsigned char comm[] = { com[0],com[1],com[2],com[3] };
		sortp(comm, 4);
		unsigned rank = find_turn(a1 * 52 + a2, comm[0] * 140608 + comm[1] * 2704 + comm[2] * 52 + comm[3]);
		//if (rank) rank += 435; //86966 / 200 = 434.83
		//rank = (int)((rank / (double)turn_rank_divde) * 5000);
		assert(rank >= 0 && rank < 5000);
		return rank;
	}
	unsigned get_river_cluster(unsigned a1, unsigned a2, unsigned char com[]) { //get turn card to 990
		unsigned char comm[] = { com[0],com[1],com[2],com[3],com[4] };
		sortp(comm, 5);
		unsigned rank = find_river(a1 * 52 + a2, comm[0] * 7311616 + comm[1] * 140608 + comm[2] * 2704 + comm[3] * 52 + comm[4]);
		//rank = (int)((rank / (double)rivern_rank_divde) * 1000);
		assert(rank >= 0 && rank < 1000);
		return rank;
	}
	unsigned get_river_cluster(unsigned char  hand[], unsigned char  com[]) { //get turn card to 990
		unsigned a1, a2;
		if (hand[0] > hand[1]) {
			a1 = hand[1];
			a2 = hand[0];
		}
		else {
			a1 = hand[0];
			a2 = hand[1];
		}
		unsigned char comm[] = { com[0],com[1],com[2],com[3],com[4] };
		sortp(comm, 5);
		unsigned rank = find_river(a1 * 52 + a2, comm[0] * 7311616 + comm[1] * 140608 + comm[2] * 2704 + comm[3] * 52 + comm[4]);
		//rank = (int)((rank / (double)rivern_rank_divde) * 1000);
		assert(rank >= 0 && rank < 1000);
		return rank;
	}
	unsigned char compute_winner(unsigned char p0_cards[2], unsigned char p1_cards[2], unsigned char community_cards[5]) {
		//assert(p0_cards[0] >= 0 && p0_cards[0] < 52);
		//assert(p0_cards[1] >= 0 && p0_cards[1] < 52);
		//assert(p1_cards[0] >= 0 && p1_cards[0] < 52);
		//assert(p1_cards[1] >= 0 && p1_cards[1] < 52);
		//assert(community_cards[0] >= 0 && community_cards[0] < 52);
		//assert(community_cards[1] >= 0 && community_cards[1] < 52);
		//assert(community_cards[2] >= 0 && community_cards[2] < 52);
		//assert(community_cards[3] >= 0 && community_cards[3] < 52);
		//assert(community_cards[4] >= 0 && community_cards[4] < 52);
		int strength1 = Maxstrength(p0_cards, community_cards);
		int strength2 = Maxstrength(p1_cards, community_cards);
		if (strength1 < strength2)
			return 0;
		else if (strength1 > strength2)
			return 1;
		else
			return 255;
	}
	inline unsigned short Maxstrength(unsigned char  cards[7]) {
		return find_strength((1ll << cards[0]) + (1ll << cards[1]) + (1ll << cards[2]) + (1ll << cards[3]) + (1ll << cards[4]) + (1ll << cards[5]) + (1ll << cards[6]));
	}
	inline unsigned short Maxstrength(unsigned char  hand[2], unsigned char  community[5]) {
		return find_strength((1ll << hand[0]) + (1ll << hand[1]) + (1ll << community[0]) + (1ll << community[1]) + (1ll << community[2]) + (1ll << community[3]) + (1ll << community[4]));
	}
	inline unsigned short Maxstrength(unsigned char c1, unsigned char c2, unsigned char  community[5]) {
		return find_strength((1ll << c1) + (1ll << c2) + (1ll << community[0]) + (1ll << community[1]) + (1ll << community[2]) + (1ll << community[3]) + (1ll << community[4]));
	}
	inline unsigned short find_strength(ll dest) {//compute 5 card hand strength
		int left = 0, right = seven_length - 1;
		while (left <= right)
		{
			int middle = (left + right) / 2;
			if (seven_keys[middle] == dest)
				return seven_strengths[middle];
			if (dest > seven_keys[middle])
				left = middle + 1;
			else
				right = middle - 1;
		}
		cout << "error hand seven cards" << dest << endl;
		throw exception();
	}
	inline unsigned find_turn(unsigned handid, unsigned communityid) {
		int left = 0, right = turn_community_total - 1;
		while (left <= right)
		{
			int middle = (left + right) / 2;
			if (turn_cluster[handid].keys[middle] == communityid)
				return turn_cluster[handid].values[middle];
			if (communityid > turn_cluster[handid].keys[middle])
				left = middle + 1;
			else
				right = middle - 1;
		}
		cout << "error hand turn cards" << handid << "," << communityid << endl;
		throw exception();
	}
	inline unsigned short find_river(unsigned handid, unsigned communityid) {
		int left = 0, right = river_community_total - 1;
		while (left <= right)
		{
			int middle = (left + right) / 2;
			if (river_cluster[handid].keys[middle] == communityid)
				return river_cluster[handid].values[middle];
			if (communityid > river_cluster[handid].keys[middle])
				left = middle + 1;
			else
				right = middle - 1;
		}
		cout << "error hand river cards" << handid << "," << communityid << endl;
		throw exception();
	}
	inline unsigned find_flop(unsigned handid, unsigned communityid) {
		int left = 0, right = flop_community_total - 1;
		while (left <= right)
		{
			int middle = (left + right) / 2;
			if (flop_cluster[handid].keys[middle] == communityid)
				return flop_cluster[handid].values[middle];
			if (communityid > flop_cluster[handid].keys[middle])
				left = middle + 1;
			else
				right = middle - 1;
		}
		cout << "error hand turn cards: " << handid << "," << communityid << endl;
		throw exception();
	}
};

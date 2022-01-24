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
#include <string.h>
#include "../poker/State.h"
#include "Node.h"
using namespace std;
ll countnode = 0;

void bias(double* oriregret, int len, double sigma[], unsigned char actionstr[], int biasid) {
	assert(len != 0);
	int regret[12];
	if (biasid == 0) {
		for (int i = 0; i < len; i++)
			regret[i] = oriregret[i];
		if (actionstr[0] == 'd' && regret[0] > 0)
			regret[0] *= 5;
	}
	else if (biasid == 1) {
		for (int i = 0; i < len; i++)
			regret[i] = oriregret[i];
		if (actionstr[0] == 'l')
			if (regret[0] > 0)
				regret[0] *= 5;
			else if (actionstr[1] == 'l')
				if (regret[1] > 0)
					regret[1] *= 5;
				else
					throw exception();
	}
	else if (biasid == 2) {
		memset(regret, 0, sizeof regret);
		for (int i = 0; i < len; i++)
			if (oriregret[i] > 0) {
				if (actionstr[i] != 'l' && actionstr[i] != 'd')
					regret[i] = oriregret[i] * 5;
				else
					regret[i] = oriregret[i];
			}
	}
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum > 0){
		for (int i = 0; i < len; i++)
			if (regret[i] > 0)
				sigma[i] = regret[i] / sum;
			else
				sigma[i] = 0;
	}
	else
		for (int i = 0; i < len; i++)
			sigma[i] = 1.0 / len;
}
void scale_regret(double* regret, int len, double sigma[], double scale = 1000) {
	assert(len != 0);
	//for (int i = 0; i < len; i++)
	//	sigma[i] = regret[i];
	scale = scale * len;
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
		else
			sum += -regret[i];
	}
	if (sum > 0)
		for (int i = 0; i < len; i++)
			if (regret[i] > 0)
				sigma[i] = regret[i] / sum * scale;
			else
				sigma[i] = regret[i] / sum * scale;
}
inline double faboslut(double a, double b) {
	double t = a - b;
	return a < 0 ? -a : a;
}
void check_saveloadsubtree(strategy_node* publicnode1[], strategy_node* publicnode2[], Pokerstate& state, int len) {
	if (state.is_terminal()) {
		assert(publicnode1[0]->action_len == 0);
		return;
	}
	if (publicnode1[0]->action_len > 100) {
		strategy_node** tempprivatenode2 = new strategy_node * [publicnode1[0]->action_len];
		strategy_node** tempprivatenode22 = new strategy_node * [publicnode2[0]->action_len];
		tempprivatenode2[0] = publicnode1[0]->actions;
		tempprivatenode22[0] = publicnode2[0]->actions;
		for (int i = 0; i < len; i++)
			assert(publicnode1[i]->action_len == publicnode2[i]->action_len);
		for (int j = 0; j < publicnode1[0]->action_len; j++) {
			tempprivatenode2[j] = publicnode1[0]->actions + j;
			tempprivatenode22[j] = publicnode2[0]->actions + j;
		}
		check_saveloadsubtree(tempprivatenode2, tempprivatenode22, state, publicnode1[0]->action_len);
		delete[] tempprivatenode2;
		delete[] tempprivatenode22;
		return;
	}
	strategy_node** tempprivatenode = new strategy_node * [len];
	strategy_node** tempprivatenode1 = new strategy_node * [len];
	unsigned char* legal_acts = new unsigned char[12];
	int actionlen = state.legal_actions(legal_acts);
	//publicroot->init_child(legal_acts, actionlen);
	for (int i = 0; i < len; i++) {
		assert(actionlen == publicnode1[i]->action_len && actionlen == publicnode2[i]->action_len);
		for (int j = 0; j < actionlen; j++) {
			assert(legal_acts[j] == publicnode1[i]->actionstr[j] && legal_acts[j] == publicnode2[i]->actionstr[j]);
			assert(publicnode1[i]->regret[j] == publicnode2[i]->regret[j]);
		}
	}
	for (int i = 0; i < actionlen; i++) {
		Pokerstate newstate = state;
		bool is_chance = newstate.take_action(legal_acts[i]);
		for (int j = 0; j < len; j++) {
			tempprivatenode[j] = publicnode1[j]->actions + i;
			tempprivatenode1[j] = publicnode2[j]->actions + i;
		}
		check_saveloadsubtree(tempprivatenode, tempprivatenode1, newstate, len);
	}
	delete[] legal_acts;
	delete[] tempprivatenode;
	delete[] tempprivatenode1;
}
void check_saveload(strategy_node* root1, strategy_node* root2, Pokerstate state) {	//check 蓝图策略 subgame tree whether action length in same state 
	int len = root1->action_len;
	assert(root1->action_len == root2->action_len);
	strategy_node* publicnode[169];
	for (int i = 0; i < len; i++)
		publicnode[i] = root1->actions + i;
	strategy_node* publicnode2[169];
	for (int i = 0; i < len; i++)
		publicnode2[i] = root2->actions + i;
	check_saveloadsubtree(publicnode, publicnode2, state, len);
}
void check_subtree(strategy_node* publicnode[], Pokerstate& state, int len) {
	if (state.is_terminal())
		return;
	if (publicnode[0]->action_len > 100) {
		strategy_node** tempprivatenode2 = new strategy_node * [publicnode[0]->action_len];
		tempprivatenode2[0] = publicnode[0]->actions;
		for (int j = 0; j < publicnode[0]->action_len; j++)
			tempprivatenode2[j] = publicnode[0]->actions + j;
		check_subtree(tempprivatenode2, state, publicnode[0]->action_len);
		delete[] tempprivatenode2;
		return;
	}
	strategy_node** tempprivatenode = new strategy_node * [len];
	unsigned char* legal_acts = new unsigned char[12];
	int actionlen = state.legal_actions(legal_acts);
	//publicroot->init_child(legal_acts, actionlen);
	for (int i = 0; i < len; i++) {
		assert(actionlen == publicnode[i]->action_len);
		for (int j = 0; j < actionlen; j++)
			assert(legal_acts[j] == publicnode[i]->actionstr[j]);
	}
	for (int i = 0; i < actionlen; i++) {
		Pokerstate newstate = state;
		bool is_chance = newstate.take_action(legal_acts[i]);
		for (int j = 0; j < len; j++)
			tempprivatenode[j] = publicnode[j]->actions + i;
		check_subtree(tempprivatenode, newstate, len);
	}
	delete[] legal_acts;
	delete[] tempprivatenode;
}
void check_subgame(strategy_node* root, Pokerstate state) {	//check 蓝图策略 subgame tree whether action length in same state 
	int len = root->action_len;
	strategy_node* publicnode[169];
	for (int i = 0; i < len; i++)
		publicnode[i] = root->actions + i;
	check_subtree(publicnode, state, len);
}
void check_subtree(subgame_node* publicnode[], Searchstate& state, int len) {//28036944
	if (state.is_terminal() || publicnode[0]->action_len == 0)
		return;
	subgame_node* tempprivatenode[1326];
	if (publicnode[0]->action_len > 100) {
		tempprivatenode[0] = publicnode[0]->actions;
		for (int j = 0; j < publicnode[0]->action_len; j++)
			tempprivatenode[j] = publicnode[0]->actions + j;
		check_subtree(tempprivatenode, state, publicnode[0]->action_len);
		return;
	}
	unsigned char* legal_acts = new unsigned char[12];
	int actionlen = state.legal_actions(legal_acts);
	//publicroot->init_child(legal_acts, actionlen);
	for (int i = 0; i < len; i++) {
		assert(actionlen == publicnode[i]->action_len);
		for (int j = 0; j < actionlen; j++)
			assert(legal_acts[j] == publicnode[i]->actionstr[j]);
	}
	for (int i = 0; i < actionlen; i++) {
		Searchstate newstate = state;
		bool is_chance = newstate.take_action(legal_acts[i]);
		for (int j = 0; j < len; j++)
			tempprivatenode[j] = publicnode[j]->actions + i;
		check_subtree(tempprivatenode, newstate, len);
	}
}
void check_subgame(subgame_node* subgameroot, Searchstate state) {	//check  subgame tree whether action length in same state 
	int len = subgameroot->action_len;
	subgame_node* publicnode[1326];
	for (int i = 0; i < len; i++)
		publicnode[i] = subgameroot->actions + i;
	check_subtree(publicnode, state, len);
}
void check_addnode_subtree(subgame_node* publicnode[], int len) {
	if (publicnode[0]->action_len == 0 || publicnode[0]->leaf)
		return;
	if (publicnode[0]->action_len > 100) {
		subgame_node** tempprivatenode = new subgame_node * [publicnode[0]->action_len];
		tempprivatenode[0] = publicnode[0]->actions;
		for (int j = 0; j < publicnode[0]->action_len; j++)
			tempprivatenode[j] = publicnode[0]->actions + j;
		check_addnode_subtree(tempprivatenode, publicnode[0]->action_len);
		delete[] tempprivatenode;
		return;
	}
	unsigned char* legal_acts = publicnode[0]->actionstr;
	int actionlen = publicnode[0]->action_len;
	//publicroot->init_child(legal_acts, actionlen);
	for (int i = 1; i < len; i++) {
		assert(actionlen == publicnode[i]->action_len);
		for (int j = 0; j < actionlen; j++)
			assert(legal_acts[j] == publicnode[i]->actionstr[j]);
	}
	subgame_node** tempprivatenode = new subgame_node * [len];
	for (int i = 0; i < actionlen; i++) {
		for (int j = 0; j < len; j++)
			tempprivatenode[j] = publicnode[j]->actions + i;
		check_addnode_subtree(tempprivatenode, len);
	}
	delete[] tempprivatenode;
}
void check_addnode(subgame_node* subgameroot) {
	int len = subgameroot->action_len;
	subgame_node* publicnode[1326];
	for (int i = 0; i < len; i++)
		publicnode[i] = subgameroot->actions + i;
	check_addnode_subtree(publicnode, len);
}
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
void bulid_subtree(strategy_node* privatenode[], Pokerstate& state, int len) {//蓝图策略树构建
	if (state.is_terminal())
		return;
	unsigned char* legal_acts = new unsigned char[12];
	int actionlen = state.legal_actions(legal_acts);
	privatenode[0]->init_child(legal_acts, actionlen);
	//if (state.betting_stage == 0)
	//	privatenode[0]->averegret = new int[actionlen];
	for (int i = 1; i < len; i++) {
		unsigned char* lacts = new unsigned char[actionlen + 1];
		for (int j = 0; j < actionlen + 1; j++)
			lacts[j] = legal_acts[j];
		privatenode[i]->init_child(lacts, actionlen);
		//if (state.betting_stage == 0)
		//	privatenode[i]->averegret = new int[actionlen];
	}
	countnode += actionlen * len;
	strategy_node** tempprivatenode = new strategy_node * [len];
	for (int i = 0; i < actionlen; i++) {
		Pokerstate newstate = state;
		bool is_chance = newstate.take_action(legal_acts[i]);
		if (is_chance) {
			int chancelen;
			if (newstate.betting_stage == 1)
				chancelen = 50000;
			else if (newstate.betting_stage == 2)
				chancelen = 5000;
			else
				chancelen = 1000;
			strategy_node** tempprivatenode2 = new strategy_node * [chancelen];
			privatenode[0]->actions[i].init_chance_node(chancelen);
			tempprivatenode2[0] = privatenode[0]->actions[i].actions;
			for (int j = 1; j < len; j++) {	//机会节点共通 赋值
				//privatenode[j]->actions[i].regret = privatenode[0]->actions[i].regret;
				privatenode[j]->actions[i].action_len = privatenode[0]->actions[i].action_len;
				privatenode[j]->actions[i].actions = privatenode[0]->actions[i].actions;
			}
			for (int j = 0; j < chancelen; j++)
				tempprivatenode2[j] = privatenode[0]->actions[i].actions + j;

			bulid_subtree(tempprivatenode2, newstate, chancelen);
			delete[] tempprivatenode2;
		}
		else {
			for (int j = 0; j < len; j++)
				tempprivatenode[j] = privatenode[j]->actions + i;
			bulid_subtree(tempprivatenode, newstate, len);
		}
	}
	delete[] tempprivatenode;
}
void bulid_preflop(strategy_node* privateroot, Pokerstate state, int len = 169) {	//bulid whole game tree
	privateroot->init_chance_node(len);
	strategy_node* privatenode[169];
	for (int i = 0; i < len; i++)
		privatenode[i] = privateroot->actions + i;
	bulid_subtree(privatenode, state, len);
	cout << "infoset node:" << countnode << endl;

}

void bulid_subtree_turn2(subgame_node* privatenode[], strategy_node* subblueprints[], Searchstate& state, int len, bool offtree, bool existmap) {//构建实时搜索子博弈树turn和river轮
	if (state.is_terminal())
		return;
	unsigned char* legal_acts = new unsigned char[12];
	int actionlen = state.legal_actions(legal_acts);
	privatenode[0]->init_child(legal_acts, actionlen);
	for (int i = 1; i < len; i++) {
		unsigned char* lacts = new unsigned char[actionlen + 1];
		for (int j = 0; j < actionlen + 1; j++)
			lacts[j] = legal_acts[j];
		privatenode[i]->init_child(lacts, actionlen);
	}
	if (!offtree)
		assert(actionlen == subblueprints[0]->action_len && existmap);
	countnode += actionlen * len;
	subgame_node* tempprivatenode[1128];
	if (existmap && actionlen == subblueprints[0]->action_len) {
		strategy_node* tempbpnodes[1128];
		for (int j = 0; j < len; j++)
			for (int k = 0; k < actionlen; k++)
				privatenode[j]->regret[k] = subblueprints[j]->averegret[k] / 10;
		for (int i = 0; i < actionlen; i++) {
			Searchstate newstate = state;
			bool is_chance = newstate.take_action(legal_acts[i]);
			if (is_chance) {
				int chancelen = 1000;
				privatenode[0]->actions[i].init_chance_node(chancelen);
				tempprivatenode[0] = privatenode[0]->actions[i].actions;
				for (int j = 1; j < len; j++) {
					privatenode[j]->actions[i].action_len = privatenode[0]->actions[i].action_len;
					privatenode[j]->actions[i].actions = privatenode[0]->actions[i].actions;
				}
				for (int j = 0; j < chancelen; j++) {
					tempprivatenode[j] = privatenode[0]->actions[i].actions + j;
					tempbpnodes[j] = subblueprints[0]->actions[i].actions + j;
				}
				bulid_subtree_turn2(tempprivatenode, subblueprints, newstate, chancelen, true, existmap);
			}
			else {
				for (int j = 0; j < len; j++) {
					tempprivatenode[j] = privatenode[j]->actions + i;
					tempbpnodes[j] = subblueprints[j]->actions + i;
				}
				bulid_subtree_turn2(tempprivatenode, tempbpnodes, newstate, len, offtree, existmap);
			}
		}
	}
	else {		//只要有一个节点动作不一样，当前节点和子节点后悔值全0
		for (int i = 0; i < actionlen; i++) {
			Searchstate newstate = state;
			bool is_chance = newstate.take_action(legal_acts[i]);
			if (is_chance) {
				int chancelen = 1000;
				privatenode[0]->actions[i].init_chance_node(chancelen);
				tempprivatenode[0] = privatenode[0]->actions[i].actions;
				for (int j = 1; j < len; j++) {
					privatenode[j]->actions[i].action_len = privatenode[0]->actions[i].action_len;
					privatenode[j]->actions[i].actions = privatenode[0]->actions[i].actions;
				}
				for (int j = 0; j < chancelen; j++)
					tempprivatenode[j] = privatenode[0]->actions[i].actions + j;
				bulid_subtree_turn2(tempprivatenode, subblueprints, newstate, chancelen, offtree, false);
			}
			else {
				for (int j = 0; j < len; j++)
					tempprivatenode[j] = privatenode[j]->actions + i;
				bulid_subtree_turn2(tempprivatenode, subblueprints, newstate, len, offtree, false);
			}
		}
	}
}
void bulid_subtree_turn(subgame_node* privateroot, strategy_node* subblueprints[], Searchstate state,bool offtree, int len = 1128) {	//构建实时搜索子博弈树turn和river轮
	countnode = 0;
	subgame_node* privatenode[1128];
	for (int i = 0; i < len; i++)
		privatenode[i] = privateroot->actions + i;
	bulid_subtree_turn2(privatenode, subblueprints, state, len, offtree, true);
	cout << "turn subgame infoset node:" << countnode << endl;
}
void bulid_subtree_river(subgame_node* subgame, Searchstate& state) {
	if (state.is_terminal())
		return;
	unsigned char* legal_acts = new unsigned char[12];
	int actionlen = state.legal_actions(legal_acts);
	subgame->init_child(legal_acts, actionlen);
	for (int i = 0; i < actionlen; i++) {
		Searchstate newstate = state;
		newstate.take_action(legal_acts[i]);
		bulid_subtree_river(subgame->actions + i, newstate);
	}
}
void bulid_subtree_river(subgame_node* subgame, strategy_node* node, Searchstate& state) {
	if (state.is_terminal())
		return;
	unsigned char* legal_acts = new unsigned char[12];
	int actionlen = state.legal_actions(legal_acts);
	assert(actionlen == node->action_len);
	subgame->init_child(legal_acts, actionlen);
	for (int j = 0; j < actionlen; j++)
		subgame->regret[j] = node->averegret[j] / 10;
	for (int i = 0; i < actionlen; i++) {
		Searchstate newstate = state;
		newstate.take_action(legal_acts[i]);
		bulid_subtree_river(subgame->actions + i, node->actions + i, newstate);
	}
}
void build_subtree_preflop(strategy_node* node, subgame_node* subgame, Searchstate& curstate, int beting_round) {
	if (curstate.is_terminal())
		return;
	else if (curstate.betting_stage > beting_round) {
		subgame->leaf = true;
		subgame->leafnode = node;
		return;
	}
	unsigned char* legal_acts = new unsigned char[13];
	int actionlen = curstate.legal_actions(legal_acts);
	subgame->init_child(legal_acts, actionlen);
	assert(actionlen == node->action_len);
	for (int i = 0; i < node->action_len; i++)
		subgame->regret[i] = node->averegret[i];
	for (int i = 0; i < actionlen; i++) {
		Searchstate newstate = curstate;
		newstate.take_action(subgame->actionstr[i]);
		build_subtree_preflop(node->actions + i, subgame->actions + i, newstate, beting_round);
	}

}
void build_subtree_flop(strategy_node* node, subgame_node* subgame, Searchstate& curstate, int beting_round,bool offtree, bool existmap = true) {
	if (curstate.is_terminal())
		return;
	else if (curstate.betting_stage > beting_round) {
		subgame->leaf = true;
		subgame->leafnode = node;
		return;
	}
	unsigned char* legal_acts = new unsigned char[12];
	int actionlen = curstate.legal_actions(legal_acts);
	subgame->init_child(legal_acts, actionlen);
	if (!offtree)
		assert(actionlen == node->action_len);
	if (actionlen <= node->action_len) {//子博弈动作小于或者等于蓝图对应节点动作
		if (actionlen == node->action_len)
			for (int j = 0; j < actionlen; j++)
				subgame->regret[j] = node->averegret[j] / 10;
		for (int i = 0; i < actionlen; i++) {
			Searchstate newstate = curstate;
			newstate.take_action(subgame->actionstr[i]);
			if (actionlen < node->action_len)
				build_subtree_flop(node->findnode(subgame->actionstr[i]), subgame->actions + i, newstate, beting_round, offtree);
			else
				build_subtree_flop(node->actions + i, subgame->actions + i, newstate, beting_round, offtree);
		}
	}
	else {					//子博弈动作比蓝图对应节点动作多
		subgame->action_len = node->action_len;		//直接等于映射的蓝图节点
		for (int i = 0; i < subgame->action_len; i++)
			subgame->actionstr[i] = node->actionstr[i];
		for (int i = 0; i < node->action_len; i++)
			subgame->regret[i] = node->averegret[i];
		//scale_regret(node->averegret, node->action_len, subgame->regret);
		for (int i = 0; i < subgame->action_len; i++) {
			Searchstate newstate = curstate;
			newstate.take_action(subgame->actionstr[i]);
			build_subtree_flop(node->actions + i, subgame->actions + i, newstate, beting_round, offtree);
		}
	}
}
void build_subgameeroot(strategy_node* node, subgame_node* subgame, Searchstate curstate) {
	assert(curstate.betting_stage == 0);
	subgame->init_chance_node(node->action_len);
	for (int j = 0; j < subgame->action_len; j++)
		build_subtree_preflop(node->actions + j, subgame->actions + j, curstate, curstate.betting_stage);
}
void build_subgameeroot(strategy_node* node, subgame_node* subgame, Searchstate curstate, Engine* engine,bool offtree, unsigned char publiccards[5] = NULL, int external_cards_len = 0, int external_cardid[] = NULL) {//建立子博弈树根
	if (curstate.betting_stage == 1) {
		subgame->init_chance_node(external_cards_len);
		int cluster;
		for (int k = 0; k < external_cards_len; k++) {
			int i = external_cardid[k] / 52, j = external_cardid[k] % 52;
			cluster = engine->get_flop_cluster(i, j, publiccards);
			build_subtree_flop(node->actions + cluster, subgame->actions + k, curstate, curstate.betting_stage, offtree);
		}
	}
	else if (curstate.betting_stage == 2) {
		int cluster;
		subgame->init_chance_node(external_cards_len);
		strategy_node* bpnodes[1128];
		for (int k = 0; k < external_cards_len; k++) {
			int i = external_cardid[k] / 52, j = external_cardid[k] % 52;
			cluster = engine->get_turn_cluster(i, j, publiccards);
			//build_subtree_flop(node->actions + cluster, subgame->actions + k, curstate, curstate.betting_stage);
			bpnodes[k] = node->actions + cluster;
		}
		bulid_subtree_turn(subgame, bpnodes, curstate, offtree, external_cards_len);
	}
	else {
		if (offtree) {
			subgame->init_chance_node(external_cards_len);
			for (int k = 0; k < external_cards_len; k++)
				bulid_subtree_river(subgame->actions + k, curstate);
		}
		else {
			subgame->init_chance_node(external_cards_len);
			for (int k = 0; k < external_cards_len; k++) {
				int i = external_cardid[k] / 52, j = external_cardid[k] % 52;
				int cluster = engine->get_river_cluster(i, j, publiccards);
				bulid_subtree_river(subgame->actions + k, node->actions + cluster, curstate);
			}
		}
		//int cluster;
		//subgame->init_chance_node(external_cards_len);
		//strategy_node* bpnodes[1128];
		//for (int k = 0; k < external_cards_len; k++) {
		//	int i = external_cardid[k] / 52, j = external_cardid[k] % 52;
		//	cluster = engine->get_river_cluster(i, j, publiccards);
		//	build_subtree_flop(node->actions + cluster, subgame->actions + k, curstate, curstate.betting_stage);
		//	//bpnodes[k] = node->actions + cluster;
		//}
	}
}

void normalize_subtree(subgame_node* subgame, Searchstate curstate, int beting_round) {//增加新动作节点时归一化子博弈树后悔值
	if (curstate.is_terminal())
		return;
	else if (curstate.betting_stage > beting_round) {
		return;
	}
	if (!subgame->frozen)
		for (int i = 0; i < subgame->action_len; i++)
			subgame->ave_strategy[i] = 0;
	for (int i = 0; i < subgame->action_len; i++) {
		Searchstate newstate = curstate;
		newstate.take_action(subgame->actionstr[i]);
		normalize_subtree(subgame->actions + i, newstate, beting_round);
	}
}
void normalize_subgameeroot(subgame_node* subgame, Searchstate curstate, int beting_round) {//增加新动作节点时归一化子博弈树后悔值
	int len = subgame->action_len;
	for (int i = 0; i < len; i++)
		normalize_subtree(subgame->actions + i, curstate, curstate.betting_stage);
}
void addnode_build_subtree(subgame_node* mapnode, subgame_node* offnode, Searchstate& curstate, int beting_round) {
	if (curstate.is_terminal())
		return;
	else if (curstate.betting_stage > beting_round) {
		offnode->leaf = true;
		if (!mapnode->leaf)
			throw exception();
		offnode->leafnode = mapnode->leafnode;
		return;
	}
	unsigned char* legal_acts = new unsigned char[12];
	int actionlen = curstate.legal_actions(legal_acts);
	offnode->init_child(legal_acts, actionlen);
	if (actionlen <= mapnode->action_len) {//离树节点的子博弈动作小于或者等于蓝图对应节点动作
		for (int i = 0; i < actionlen; i++)
			offnode->regret[i] = mapnode->regret[i];
		for (int i = 0; i < actionlen; i++) {
			Searchstate newstate = curstate;
			newstate.take_action(offnode->actionstr[i]);
			addnode_build_subtree(mapnode->findnode(offnode->actionstr[i]), offnode->actions + i, newstate, beting_round);
		}
	}
	else {					//离树节点的子博弈动作比蓝图对应节点动作多
		offnode->action_len = mapnode->action_len;		//直接等于映射的蓝图节点
		for (int i = 0; i < offnode->action_len; i++) {
			offnode->actionstr[i] = mapnode->actionstr[i];
			offnode->regret[i] = mapnode->regret[i];
		}
		for (int i = 0; i < offnode->action_len; i++) {
			Searchstate newstate = curstate;
			newstate.take_action(offnode->actionstr[i]);
			addnode_build_subtree(mapnode->actions + i, offnode->actions + i, newstate, beting_round);
		}
	}
}
void build_small_subtree(subgame_node* privatenode[], Searchstate& state, int len) {
	if (state.is_terminal())
		return;
	unsigned char* legal_acts = new unsigned char[12];
	int actionlen = state.legal_actions(legal_acts);
	privatenode[0]->init_child(legal_acts, actionlen);
	for (int i = 1; i < len; i++) {
		unsigned char* lacts = new unsigned char[actionlen + 1];
		for (int j = 0; j < actionlen + 1; j++)
			lacts[j] = legal_acts[j];
		privatenode[i]->init_child(lacts, actionlen);
	}
	countnode += actionlen * len;
	for (int i = 0; i < actionlen; i++) {
		Searchstate newstate = state;
		bool is_chance = newstate.take_action(legal_acts[i]);
		if (is_chance) {
			int chancelen;
			if (newstate.betting_stage == 3)
				chancelen = 1000;
			else if (newstate.betting_stage == 2)
				chancelen = 5000;
			else if (newstate.betting_stage == 1)
				chancelen = 50000;
			privatenode[0]->actions[i].init_chance_node(chancelen);
			for (int j = 1; j < len; j++) {
				//privatenode[j]->actions[i].regret = privatenode[0]->actions[i].regret;
				privatenode[j]->actions[i].action_len = privatenode[0]->actions[i].action_len;
				privatenode[j]->actions[i].actions = privatenode[0]->actions[i].actions;
			}
			subgame_node** tempprivatenode = new subgame_node * [chancelen];
			tempprivatenode[0] = privatenode[0]->actions[i].actions;
			for (int j = 1; j < chancelen; j++)
				tempprivatenode[j] = privatenode[0]->actions[i].actions + j;

			build_small_subtree(tempprivatenode, newstate, chancelen);
			delete[] tempprivatenode;
		}
		else {
			subgame_node** tempprivatenode = new subgame_node * [len];
			for (int j = 0; j < len; j++)
				tempprivatenode[j] = privatenode[j]->actions + i;
			build_small_subtree(tempprivatenode, newstate, len);
			delete[] tempprivatenode;
		}
	}
}
void addnode_bysubgame(subgame_node* subgameroot, vector<unsigned char> hisact, Searchstate curstate, int beting_round, unsigned char offtreeact, unsigned char treeact, int after_action_pot) {//添加离树动作
	subgame_node* publicinfoset[1176];
	for (int i = 0; i < subgameroot->action_len; i++)
		publicinfoset[i] = subgameroot->actions + i;
	int len = hisact.size();
	curstate.take_action(offtreeact);
	for (int j = 0; j < subgameroot->action_len; j++) {
		for (int i = 0; i < len; i++)
			publicinfoset[j] = publicinfoset[j]->findnode(hisact[i]);
		subgame_node* oldaction = publicinfoset[j]->actions;				//旧child node[]
		publicinfoset[j]->actionstr[publicinfoset[j]->action_len] = offtreeact;	//新增动作节点增加动作char
		publicinfoset[j]->action_len++;											//新增动作节点增加标志child node 长度
		subgame_node* newactions = new subgame_node[publicinfoset[j]->action_len];
		for (int q = 0; q < publicinfoset[j]->action_len - 1; q++)				//新的action_len-1 节点从旧的child node中赋值过来
			newactions[q] = oldaction[q];
		publicinfoset[j]->actions = newactions;
		delete []oldaction;							//释放旧child node内存
		double* oldregrets = publicinfoset[j]->regret;				//regret 重新赋值
		double* old_averegrets = publicinfoset[j]->ave_strategy;				//ave_regret 重新赋值
		double* newregrets = new double[publicinfoset[j]->action_len]{ 0 };
		double* new_averegrets = new double[publicinfoset[j]->action_len]{ 0 };
		for (int q = 0; q < publicinfoset[j]->action_len - 1; q++) {
			newregrets[q] = oldregrets[q];
			//new_averegrets[q] = old_averegrets[q];
		}
		publicinfoset[j]->regret = newregrets;
		publicinfoset[j]->ave_strategy = new_averegrets;
		if (beting_round < 2 && after_action_pot >= 20000) {
			publicinfoset[j] = publicinfoset[j]->actions + publicinfoset[j]->action_len - 1;
			unsigned char* tt = new unsigned char[3];
			tt[0] = 'd';
			tt[1] = 'n';
			publicinfoset[j]->init_child(tt, 2);
			assert(publicinfoset[j]->action_len == 2);
			publicinfoset[j] = publicinfoset[j]->actions + 1;
			unsigned char* tt1 = new unsigned char[3];
			tt1[0] = 'd';
			tt1[1] = 'l';
			publicinfoset[j]->init_child(tt1, 2);
		}
		else {
			for (int tt = 0; tt < publicinfoset[j]->action_len - 1; tt++)		//映射动作的后悔值赋值给新增动作
				if (publicinfoset[j]->actionstr[tt] == treeact) {
					publicinfoset[j]->regret[publicinfoset[j]->action_len - 1] = publicinfoset[j]->regret[tt];
					break;
					//publicinfoset[j]->ave_strategy[publicinfoset[j]->action_len - 1] = publicinfoset[j]->ave_strategy[tt];
				}
		}
		delete[]oldregrets;
		delete[]old_averegrets;
		//build_subtree(publicinfoset[j]->findnode(treeact), publicinfoset[j]->actions + publicinfoset[j]->action_len - 1, curstate, beting_round);
	}
	if (beting_round < 2 && after_action_pot >= 20000)
		return;
	else if (beting_round <= 1 && after_action_pot < 18000)		//博弈规模过大
		for (int j = 0; j < subgameroot->action_len; j++)
			addnode_build_subtree(publicinfoset[j]->findnode(treeact), publicinfoset[j]->actions + publicinfoset[j]->action_len - 1, curstate, beting_round);
	else {					//博弈规模足够小
		for (int j = 0; j < subgameroot->action_len; j++)
			publicinfoset[j] = publicinfoset[j]->actions + publicinfoset[j]->action_len - 1;
		build_small_subtree(publicinfoset, curstate, subgameroot->action_len);
	}
}
subgame_node* findsubgamenode(subgame_node* subgame, vector<unsigned char> hisact) {//释放子博弈树内存
	int len = hisact.size();
	for (int i = 0; i < len; i++)
		subgame = subgame->findnode(hisact[i]);
	return subgame;
}
void dfs_delete(subgame_node* subgamenode, bool flag = true) {
	if (subgamenode->action_len == 0 || subgamenode->actions[0].action_len == 0)
		return;
	if (flag && subgamenode->action_len > 100) {
		dfs_delete((subgamenode->actions), true);
		for (int i = 1; i < subgamenode->action_len; i++)
			dfs_delete((subgamenode->actions + i), false);
	}
	else if (subgamenode->action_len > 0 && subgamenode->action_len < 100) {
		for (int i = 0; i < subgamenode->action_len; i++)
			dfs_delete((subgamenode->actions + i), flag);
	}
	else
		return;
	if (subgamenode->action_len > 100) {
		delete[] subgamenode->actions;
	}
	else if (subgamenode->action_len > 0) {
		delete[] subgamenode->actions;
		delete[] subgamenode->actionstr;
		delete[] subgamenode->regret;
		delete[] subgamenode->ave_strategy;
	}
	subgamenode->action_len = 0;
}

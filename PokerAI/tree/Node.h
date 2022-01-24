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
#include <assert.h>
#include <exception>
using namespace std;
struct strategy_node {		//info set node 
	//bool vis;
	int action_len = 0;
	unsigned char* actionstr ;
	double* regret;
	//float* cfv1;
	//float* cfv2;
	double* averegret;
	strategy_node* actions ;
	strategy_node() :action_len(0){}
	strategy_node* findnode(unsigned char action) {
		for (int i = 0; i < action_len; i++) {
			if (*(actionstr + i) == action)
				return (actions + i);
		}
		throw std::exception();
	}
	int findindex(unsigned char action) {
		for (int i = 0; i < action_len; i++) {
			if (*(actionstr + i) == action)
				return i;
		}
		throw std::exception();
	}
	void init_child(unsigned char* action_str, int _action_len) {
		actionstr = action_str;
		actions = new strategy_node[_action_len]();
		regret = new double[_action_len] {0};
		//cfv1 = new float[_action_len] {0};
		averegret = new double[_action_len] {0};
		action_len = _action_len;
		
	}
	void init_chance_node(int initlen) {

		actions = new strategy_node[initlen]();
		action_len = initlen;
	}
};
void calculate_strategy(double* oriregret, int len, double sigma[]) {
	assert(len != 0);
	int regret[12];
	for (int i = 0; i < len; i++)
		regret[i] = oriregret[i];
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum > 0)
		for (int i = 0; i < len; i++)
			if (regret[i] > 0)
				sigma[i] = regret[i] / sum;
			else
				sigma[i] = 0;
	else
		for (int i = 0; i < len; i++)
			sigma[i] = 1.0 / len;
}
double calculate_strategy(double* regret, int len, int actioni) {
	assert(len != 0);
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum > 0){
		if (regret[actioni] > 0)
			return regret[actioni] / sum;
		else
			return 0;
	}
	else
		return 1.0 / len;
}
double calculate_strategy(int* regret, int len, int actioni) {
	assert(len != 0);
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum > 0) {
		if (regret[actioni] > 0)
			return regret[actioni] / sum;
		else
			return 0;
	}
	else
		return 1.0 / len;
}

struct subgame_node {		//info set node
	bool frozen, leaf;	//leaf:深度受限叶节点，frozen：冻结这个节点策略不更新
	int action_len = 0;
	unsigned char* actionstr ;
	double* regret;
	double* ave_strategy;
	subgame_node* actions ;
	strategy_node* leafnode ;
	double* expolitvalues;
	subgame_node() :action_len(0), frozen(false), leaf(false), actions(NULL){}
	subgame_node* findnode(unsigned char action) {
		if (action == 'k')
			action = 'l';
		for (int i = 0; i < action_len; i++) {
			if (*(actionstr + i) == action)
				return (actions + i);
		}
		cout << "not this actions:" << action << endl;
		throw exception();
	}
	int findindex(unsigned char action) {
		if (action == 'k')
			action = 'l';
		for (int i = 0; i < action_len; i++) {
			if (*(actionstr + i) == action)
				return i;
		}
		cout << "not this actions:" << action << endl;
		throw exception();
	}
	void init_child(unsigned char* action_str, int _action_len) {
		//if (action_len == 0) {
			actionstr = action_str;
			ave_strategy = new double[_action_len] {0};
			actions = new subgame_node[_action_len]();
			regret = new double[_action_len] {0};
			action_len = _action_len;
		//}
	}
	void init_chance_node(int initlen) {
		//if (action_len == 0) {
			actions = new subgame_node[initlen]();
			expolitvalues = new double[initlen] {0};
			action_len = initlen;
		//}
	}
};
double calculate_strategy_action(int* oriregret, int len, int index) {
	assert(len != 0 && index < len);
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (oriregret[i] > 0) 
			sum += oriregret[i];
	}
	if (sum == 0) {//确保后悔值一定有正数
		return 1.0 / len;
	}
	//当前动作后悔值要正数
	if (oriregret[index] > 0) 
		return oriregret[index] / sum;
	else
		return 0;
}
double calculate_strategy_action(int* oriregret, int len, unsigned char actionstr[], int biasid , int index) {
	assert(len != 0);
	int regret[12];
	for (int i = 0; i < len; i++)
		regret[i] = oriregret[i];
	if (biasid == 1) {
		if (actionstr[0] == 'd' && regret[0] > 0)
			regret[0] *= 5;
	}
	else if (biasid == 2) {
		if (actionstr[0] == 'l') {
			if (regret[0] > 0)
				regret[0] *= 5;
		}
		else if (actionstr[1] == 'l') {
			if (regret[1] > 0)
				regret[1] *= 5;
		}
		else
			throw exception();
	}
	else if (biasid == 3) {
		for (int i = 0; i < len; i++)
			if (oriregret[i] > 0) {
				if (actionstr[i] != 'l' && actionstr[i] != 'd')
					regret[i] = oriregret[i] * 5;
				else
					regret[i] = oriregret[i];
			}
	}
	else if (biasid == 4) {
		for (int i = 0; i < len; i++)
			if (oriregret[i] > 0) {
				if (actionstr[i] != 'l' && actionstr[i] != 'd')
					regret[i] = oriregret[i] * 10;
				else
					regret[i] = oriregret[i];
			}
	}
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum == 0) {//确保后悔值一定有正数
		return 1.0 / len;
	}
	if (regret[index] > 0)
		return regret[index] / sum;
	else
		return 0;
}
double calculate_strategy_action(double* oriregret, int len, unsigned char actionstr[], int biasid, int index) {
	assert(len != 0);
	if (oriregret[index] <= 0)
		return 0;
	double regret[12];
	for (int i = 0; i < len; i++)
		regret[i] = oriregret[i];
	if (biasid == 1) {
		if (actionstr[0] == 'd' && regret[0] > 0)
			regret[0] *= 5;
	}
	else if (biasid == 2) {
		if (actionstr[0] == 'l') {
			if (regret[0] > 0)
				regret[0] *= 5;
		}
		else if (actionstr[1] == 'l') {
			if (regret[1] > 0)
				regret[1] *= 5;
		}
		else
			throw exception();
	}
	else if (biasid == 3) {
		for (int i = 0; i < len; i++)
			if (oriregret[i] > 0) {
				if (actionstr[i] != 'l' && actionstr[i] != 'd')
					regret[i] = oriregret[i] * 5;
				else
					regret[i] = oriregret[i];
			}
	}
	else if (biasid == 4) {
		for (int i = 0; i < len; i++)
			if (oriregret[i] > 0) {
				if (actionstr[i] != 'l' && actionstr[i] != 'd')
					regret[i] = oriregret[i] * 10;
				else
					regret[i] = oriregret[i];
			}
	}
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum == 0) {//确保后悔值一定有正数
		return 1.0 / len;
	}
	if (regret[index] > 0)
		return regret[index] / sum;
	else
		return 0;
}

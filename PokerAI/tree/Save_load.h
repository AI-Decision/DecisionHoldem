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
#include "Node.h"
ofstream fout;

void dfs_write(strategy_node* privatenode[], int clusterlen) {		//save strategy
	int len = privatenode[0]->action_len;
	fout.write((char*)&len, sizeof(int));
	if (len == 0)
		return;
	else if (len < 100 && len > 0) {
		fout.write((const char*)privatenode[0]->actionstr, sizeof(unsigned char) * len);
		for (int i = 0; i < clusterlen; i++) {
			assert(privatenode[i]->action_len == len);
			fout.write((const char*)privatenode[i]->regret, sizeof(double) * len);
			fout.write((const char*)privatenode[i]->averegret, sizeof(double) * len);
		}
		strategy_node** tempprivatenode = new strategy_node * [clusterlen];
		for (int i = 0; i < len; i++) {
			for (int j = 0; j < clusterlen; j++)
				tempprivatenode[j] = privatenode[j]->actions + i;
			dfs_write(tempprivatenode, clusterlen);
		}
		delete[] tempprivatenode;
	}
	else if (len > 100) {
		strategy_node** privatenode2 = new strategy_node * [len];
		for (int j = 0; j < len; j++)
			privatenode2[j] = privatenode[0]->actions + j;
		dfs_write(privatenode2, len);
		delete[] privatenode2;
		//fout.write((const char*)cnode->regret, sizeof(int) * len);
	}
	else throw exception();
}
void dump(strategy_node* root, char filename[]) {
	fout.open(filename, ios::out | ios::binary);
	strategy_node* privatenode[169];
	for (int i = 0; i < 169; i++)
		privatenode[i] = root->actions + i;
	dfs_write(privatenode, 169);
	fout.close();
}
ifstream fin;
int countterminal = 0;
void bulid_bluestrategy(strategy_node* privatenode[], int clusterlen) {
	int len;
	if (fin.eof())
		return;
	fin.read((char*)&len, sizeof(int));
	if (len <= 0) {
		for (int i = 0; i < clusterlen; i++)
			privatenode[i]->action_len = 0;
		countterminal++;
		return; 
	}
	else if (len < 100) {
		unsigned char* actionstr = new unsigned char[len];// (char*)malloc(sizeof(char) * cnode->action_len);
		fin.read((char*)actionstr, len);
		for (int i = 0; i < clusterlen; i++) {
			privatenode[i]->actionstr = new unsigned char[len];
			for (int j = 0; j < len; j++)
				privatenode[i]->actionstr[j] = actionstr[j];
			privatenode[i]->init_child(privatenode[i]->actionstr, len);
			assert(len == privatenode[i]->action_len);
			fin.read((char*)privatenode[i]->regret, sizeof(double) * len);
			fin.read((char*)privatenode[i]->averegret, sizeof(double) * len);
			double sum = 0;
			for (int k = 0; k < len; k++)
				sum += privatenode[i]->averegret[k];
			assert(sum > 0);
		}
		strategy_node** tempprivatenode = new strategy_node * [clusterlen];
		for (int i = 0; i < len; i++) {
			for (int j = 0; j < clusterlen; j++)
				tempprivatenode[j] = privatenode[j]->actions + i;
			bulid_bluestrategy(tempprivatenode, clusterlen);
		}
		delete[] tempprivatenode;
	}
	else if (len >= 100) {
		privatenode[0]->init_chance_node(len);
		for (int j = 1; j < clusterlen; j++) {
			privatenode[j]->action_len = len;
			privatenode[j]->actions = privatenode[0]->actions;
		}
		strategy_node** privatenode2 = new strategy_node * [len];
		for (int j = 0; j < len; j++)
			privatenode2[j] = privatenode[0]->actions + j;
		bulid_bluestrategy(privatenode2, len);
		delete[] privatenode2;
	}
	else throw exception();
}
void load(strategy_node* root, char filename[]) {//316174
	fin.open(filename, ios::in | ios::binary);
	root->init_chance_node(169);
	strategy_node* privatenode[169];
	for (int i = 0; i < 169; i++)
		privatenode[i] = root->actions + i;
	countterminal = 0;
	bulid_bluestrategy(privatenode, 169);
	cout << "countterminal:" << countterminal << endl;
	fin.close();
}

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
#include <string.h>
#include <map>
#include "../poker/State.h"
map<string, bool> mapp;
map<string, bool>::iterator it;
//int dotend = 102000, dotstart=100000, dottimes = 0;
int dotend = 1000, dotstart = 0, dottimes = 0;
void write2dot(strategy_node* tree, FILE* fw, char his[], int cur, char clusters[])
{
	if (dottimes > dotend)
		return;
	if (tree->action_len == 0)
		return;
	//tree->vis = true;
	char ori[115];
	for (int i = 0; i <= cur; i++)
		ori[i] = his[i];
	if (cur != 1)
		strcat(ori, clusters);
	//if (!strcmp(ori, "TCllCllCll2"))
	//	cout << endl;
	int ans;
	if (tree->action_len > 5) {
		for (int i = 0; i < tree->action_len; i++) {
			his[cur] = 'C';
			his[cur + 1] = '\0';
			ans = cur + 1;
			if ((tree->actions + i)->action_len > 0) {
				int tp = i + 1;
				if (tp >= 100)
					clusters[0] = tp / 100 + '0', clusters[1] = (tp / 10) % 10 + '0', clusters[2] = tp % 10 + '0', clusters[3] = '\0';
				else if (tp >= 10)
					clusters[0] = tp / 10 + '0', clusters[1] = tp % 10 + '0', clusters[2] = '\0';
				else
					clusters[0] = tp + '0', clusters[1] = '\0';
				char dest[150];
				int p = 0;
				for (; p < ans; p++)
					dest[p] = his[p];
				dest[p] = '\0';
				strcat(dest, clusters);
				if (dottimes >= dotstart) {
					string nodename = dest;
					it = mapp.find(nodename);
					if (it == mapp.end()) {
						mapp[nodename] = true;
						fprintf(fw, "%s [label = \"*\" color = red, style = filled];\n", dest);
					}

					fprintf(fw, "%s -> %s [label = \"%d||%d\" ];\n", ori, dest, tp, tree->regret[i]);
				}
				dottimes++;
				if (dottimes > dotend)
					return;
				if (tree->action_len == 169) {
					for (int k = 0; k < 169; k++)
						if (tree->regret[i] < tree->regret[k])
							i = k;
				}
				char tm[4];
				strcpy(tm, clusters);
				//if (!(tree->actions + i)->vis) 
				write2dot((tree->actions + i), fw, his, ans, tm);
			}
		}
	}
	else {

		for (int i = 0; i < tree->action_len; i++) {
			his[cur] = tree->actionstr[i];
			his[cur + 1] = '\0';
			char dest[150];
			int p = 0;
			for (; p < cur + 1; p++)
				dest[p] = his[p];
			dest[p] = '\0';
			strcat(dest, clusters);
			if (dottimes >= dotstart) {
				fprintf(fw, "%s [label = \"*\" color = black, style = filled];\n", dest);
				fprintf(fw, "%s -> %s [label = \"%c|%d\" ];\n", ori, dest, his[cur], tree->regret[i]);
			}
			dottimes++;
			if (dottimes > dotend)
				return;
			char tm[4];
			strcpy(tm, clusters);
			write2dot((tree->actions + i), fw, his, cur + 1, tm);
		}
	}
}
void visualization(strategy_node* tree, const char* filename)
{
	FILE* fw;
	char his[115] = "T";
	char clusters[4];
	if (NULL == (fw = fopen(filename, "w")))
	{
		printf("open file error");
		exit(0);
	}
	fprintf(fw, "digraph\n{\nnode [shape = Mrecord, style = filled, color = black, fontcolor = white];\n");
	fprintf(fw, "%s [color = red, style = filled];\n", his);
	write2dot(tree, fw, his, 1, clusters);
	fprintf(fw, "}");
	fclose(fw);
}
void write2dotpublic(strategy_node* tree, FILE* fw, char his[], int cur, Pokerstate state)
{
	if (tree->action_len == 0)
		return;
	char ori[115];
	for (int i = 0; i <= cur; i++)
		ori[i] = his[i];
	char* la = new char[20];
	for (int i = 0; i < tree->action_len; i++) {
		if (tree->actionstr[i] > 0 && tree->actionstr[i] <= 16)
			his[cur] = tree->actionstr[i] / 2 + '0';
		else
			his[cur] = tree->actionstr[i];
		his[cur + 1] = '\0';
		int orp = state.player_i_index;
		Pokerstate newstate = state;
		bool ischanced = newstate.take_action(tree->actionstr[i]);
		if (ischanced)
			fprintf(fw, "%s [label = \"%d\" color = red, style = filled];\n", his, newstate.table.players[orp].n_bet_chips());
		else
			fprintf(fw, "%s [label = \"%d\" color = black, style = filled];\n", his, newstate.table.players[orp].n_bet_chips());
		if (tree->actionstr[i] > 0 && tree->actionstr[i] <= 16)
			fprintf(fw, "%s -> %s [label = \"R%d\" ];\n", ori, his, tree->actionstr[i]);
		else
			fprintf(fw, "%s -> %s [label = \"%c\" ];\n", ori, his, tree->actionstr[i]);


		write2dotpublic((tree->actions + i), fw, his, cur + 1, newstate);
	}
}
void visualizationpublic(strategy_node* tree, const char* filename)
{
	Player players[] = { Player(20000),Player(20000) };
	PokerTable table(2, players);
	strategy_node* pref[2];
	Pokerstate state(table);
	state.reset_game();
	FILE* fw;
	char his[115] = "T";
	if (NULL == (fw = fopen(filename, "w")))
	{
		printf("open file error");
		throw exception();
	}
	fprintf(fw, "digraph\n{\nnode [shape = Mrecord, style = filled, color = black, fontcolor = white,width=0.3];\n");
	fprintf(fw, "%s [color = red, style = filled];\n", his);
	write2dotpublic(tree, fw, his, 1, state);
	fprintf(fw, "}");
	fclose(fw);
}
extern unsigned char preflop_cluster[prim_preflop];
void getcardid(int k, char rs[]) {
	assert(k >= 0 && k < 169);
	int cur = -1;
	for (int i = 0; i < 52; i++)
		for (int j = i + 1; j < 52; j++)
			if (preflop_cluster[i * 52 + j] == k) {
				cur = i * 52 + j; goto css;
			}
css:assert(cur != -1);
	int a1 = cur / 52, a2 = cur % 52;
	char suits[] = "scdh";
	char ranks[] = "23456789TJQKA";
	rs[0] = ranks[a1 % 13];
	rs[1] = suits[a1 / 13];
	rs[2] = '|';
	rs[3] = ranks[a2 % 13];
	rs[4] = suits[a2 / 13];
	rs[5] = '\0';
}
void write2dotsearch(strategy_node* tree, FILE* fw, char his[], int cur, int depth) {
	//if (tree->leaf == true)
	//	return;
	if (depth > 2)
		return;
	if (tree->action_len > 100 || tree->action_len <= 0)
		return;
	//if (dottimes > dotend)
	//	return;
	char ori[115];
	for (int i = 0; i <= cur; i++)
		ori[i] = his[i];
	int ans;
	double sigma[15];
	calculate_strategy(tree->averegret, tree->action_len, sigma);
	for (int i = 0; i < tree->action_len; i++) {
		if (depth == 2 && sigma[i] == 0)
			continue;
		ans = 0;
		char tmp[10];
		if (tree->actionstr[i] <= 80 || tree->actionstr[i] == 160) {
			tmp[ans++] = 'r';
			if ((tree->actionstr[i] / 2) >= 10)
				tmp[ans++] = tree->actionstr[i] / 2 / 10 + '0', tmp[ans++] = (tree->actionstr[i] / 2) % 10 + '0';
			else
				tmp[ans++] = (tree->actionstr[i] / 2) + '0';
		}
		else if (tree->actionstr[i] >= 129 && tree->actionstr[i] < 160)
			tmp[ans++] = 'R';
		else
			tmp[ans++] = tree->actionstr[i];
		tmp[ans] = '\0';
		strcat(his, tmp);
		//if (dottimes >= dotstart) {
		fprintf(fw, "%s [label = \"*\" color = black, style = filled];\n", his);
		fprintf(fw, "%s -> %s [label = \"%s|%.2lf|%.2lf\" ];\n", ori, his, tmp, tree->averegret[i], sigma[i] * 100);
		//}
		//dottimes++;
		//if (dottimes > dotend)
		//	return;
		write2dotsearch((tree->actions + i), fw, his, cur + ans, depth + 1);
		his[cur] = '\0';
	}

}
void visualizationsearch(strategy_node* tree, const char* filename)
{
	FILE* fw;
	char his[115] = "c";
	if (NULL == (fw = fopen(filename, "w")))
	{
		printf("open file error");
		exit(0);
	}
	fprintf(fw, "digraph\n{\nnode [shape = Mrecord, style = filled, color = black, fontcolor = white,width=0.3];\n");
	fprintf(fw, "%s [color = red, style = filled];\n", his);
	char ori[115] = "c";
	int ans, cur = 1;
	if (tree->action_len > 100) {
		for (int i = 0; i < tree->action_len; i++) {//tree->action_len
			his[cur] = 'C';
			ans = cur;
			int tp = i + 1;
			if (tp >= 100)
				his[ans++] = tp / 100 + '0', his[ans++] = (tp / 10) % 10 + '0', his[ans++] = tp % 10 + '0', his[ans] = '\0';
			else if (tp >= 10)
				his[ans++] = tp / 10 + '0', his[ans++] = tp % 10 + '0', his[ans] = '\0';
			else
				his[ans++] = tp + '0', his[ans] = '\0';
			char rs[50];
			getcardid(i, rs);

			fprintf(fw, "%s -> %s [label = \"%s\" ];\n", ori, his, rs);
			fprintf(fw, "%s [color = red, style = filled];\n", his);

			fprintf(fw, "subgraph cluster_%d {\n", tp);
			fprintf(fw, "style = \"dashed\";\n");
			write2dotsearch((tree->actions + i), fw, his, ans, 1);
			fprintf(fw, "}\n");
		}
	}
	//write2dotsearch(tree, fw, his, 1);
	fprintf(fw, "}");
	fclose(fw);
}
/*void visualizationsearch_betting234(subgame_node* tree, const char* filename, int cardsid[], double cardsweight[], int external_cards_len, int cardid_to_index[])
{
	double clusterw[1000];
	vector<int> vec[1000];
	memset(clusterw, -1, sizeof clusterw);
	for (int i = 0; i < external_cards_len; i++) {
		assert(cardid_to_index[cardsid[i]] > 0 && cardid_to_index[cardsid[i]] < 1000);
		vec[cardid_to_index[cardsid[i]]].push_back(cardsid[i]);
		clusterw[cardid_to_index[cardsid[i]]] = cardsweight[i];
	}
	FILE* fw;
	char his[115] = "c";
	if (NULL == (fw = fopen(filename, "w")))
	{
		printf("open file error");
		exit(0);
	}
	fprintf(fw, "digraph\n{\nnode [shape = Mrecord, style = filled, color = black, fontcolor = white,width=0.3];\n");
	fprintf(fw, "%s [color = red, style = filled];\n", his);
	char ori[115] = "c";
	int ans, cur = 1;
	char suits[] = "scdh";
	char ranks[] = "23456789TJQKA";
	if (tree->action_len > 100) {
		for (int i = 0; i < 1000; i++) {//tree->action_len
			if (vec[i].size() == 0)
				continue;
			his[cur] = 'C';
			ans = cur;
			int tp = i + 1;
			if (tp >= 100)
				his[ans++] = tp / 100 + '0', his[ans++] = (tp / 10) % 10 + '0', his[ans++] = tp % 10 + '0', his[ans] = '\0';
			else if (tp >= 10)
				his[ans++] = tp / 10 + '0', his[ans++] = tp % 10 + '0', his[ans] = '\0';
			else
				his[ans++] = tp + '0', his[ans] = '\0';
			char rs[500];
			for (int j = 0; j < vec[i].size(); j++) {
				unsigned char a1 = vec[i][j] / 52;
				unsigned char a2 = vec[i][j] % 52;
				rs[j * 6 + 0] = ranks[a1 % 13];
				rs[j * 6 + 1] = suits[a1 / 13];
				rs[j * 6 + 2] = ' ';
				rs[j * 6 + 3] = ranks[a2 % 13];
				rs[j * 6 + 4] = suits[a2 / 13];
				rs[j * 6 + 5] = '|';
			}
			rs[vec[i].size() * 6] = '\0';
			fprintf(fw, "%s -> %s [label = \"%s[%.2lf]\" ];\n", ori, his, rs, clusterw[i] * 100);
			fprintf(fw, "%s [color = red, style = filled];\n", his);
			fprintf(fw, "subgraph cluster_%d {\n", tp);
			fprintf(fw, "style = \"dashed\";\n");
			write2dotsearch(tree->actions + i, fw, his, ans, 1);
			fprintf(fw, "}\n");
		}
	}
	//write2dotsearch(tree, fw, his, 1);
	fprintf(fw, "}");
	fclose(fw);
}
void visualizationsearch_bettingsub234(subgame_node* tree, const char* filename, int cardsid[], double cardsweight[],int external_cards_len, int cardid_to_index[])
{
	FILE* fw;
	char his[115] = "c";
	if (NULL == (fw = fopen(filename, "w")))
	{
		printf("open file error");
		exit(0);
	}
	fprintf(fw, "digraph\n{\nnode [shape = Mrecord, style = filled, color = black, fontcolor = white,width=0.3];\n");
	fprintf(fw, "%s [color = red, style = filled];\n", his);
	char ori[115] = "c";
	int ans, cur = 1;
	char suits[] = "scdh";
	char ranks[] = "23456789TJQKA";
	if (tree->action_len > 100) {
		for (int i = 0; i < external_cards_len; i++) {//tree->action_len
			his[cur] = 'C';
			ans = cur;
			int tp = i + 1;
			if (tp >= 100)
				his[ans++] = tp / 100 + '0', his[ans++] = (tp / 10) % 10 + '0', his[ans++] = tp % 10 + '0', his[ans] = '\0';
			else if (tp >= 10)
				his[ans++] = tp / 10 + '0', his[ans++] = tp % 10 + '0', his[ans] = '\0';
			else
				his[ans++] = tp + '0', his[ans] = '\0';
			char rs[50];
			unsigned char a1 = cardsid[i] / 52;
			unsigned char a2 = cardsid[i] % 52;
			rs[0] = ranks[a1 % 13];
			rs[1] = suits[a1 / 13];
			rs[2] = '|';
			rs[3] = ranks[a2 % 13];
			rs[4] = suits[a2 / 13];
			rs[5] = '\0';
			fprintf(fw, "%s -> %s [label = \"%s|%.2lf\" ];\n", ori, his, rs, cardsweight[i]*100);
			fprintf(fw, "%s [color = red, style = filled];\n", his);
			fprintf(fw, "subgraph cluster_%d {\n", tp);
			fprintf(fw, "style = \"dashed\";\n");
			write2dotsearch(tree->actions + cardid_to_index[cardsid[i]], fw, his, ans, 1);
			fprintf(fw, "}\n");
		}
	}
	//write2dotsearch(tree, fw, his, 1);
	fprintf(fw, "}");
	fclose(fw);
}*/

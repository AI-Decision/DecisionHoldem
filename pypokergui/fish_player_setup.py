import sys
from ctypes import *
import math

class State():
    def __init__(self):
        self.player_index = 0
        self.betting_stage = 0
        self.curbetting_round_actions = 0
        self.cur_chance_node = False
        self.had_allin = False
        self.players_chips=[19900,19950]
        self.last_round_bets = 20000
        self.reset_round()

    def reset_round(self):
        if self.betting_stage == 0:
            self.player_index = 1
        else:
            self.player_index = 0
        self.curbetting_round_actions = 0

    def move_next_player(self):
        self.player_index ^= 1
    def Next_stage(self):
        self.betting_stage += 1
        self.reset_round()
    def apply_action(self,actionstr):
        if (actionstr == 'call' or actionstr == 'check'):
            need_call_chips = self.players_chips[self.player_index] - self.players_chips[self.player_index ^ 1]
            self.players_chips[self.player_index] -= need_call_chips
        if (actionstr == 'call' or actionstr == 'check') and self.curbetting_round_actions > 0:
            if self.had_allin:
                self.betting_stage = 4
                self.curbetting_round_actions += 1
                self.players_chips[self.player_index] = 0
                return
            self.Next_stage()
            if self.betting_stage < 4:
                self.cur_chance_node = True
                self.last_round_bets = self.players_chips[0]
                return
        elif actionstr[:5] == 'raise':
            last_action_chips = self.last_round_bets - self.players_chips[self.player_index]
            need_raise_chips = int(actionstr[6:]) - last_action_chips
            self.players_chips[self.player_index] -= need_raise_chips
            if self.players_chips[self.player_index] == 0:
                self.had_allin = True
            elif self.players_chips[self.player_index] < 0:
                raise Exception('error raise',actionstr)
        elif actionstr == 'allin':
            self.players_chips[self.player_index] = 0
            self.had_allin = True
            if self.players_chips[self.player_index] != 0:
                raise Exception('error allin', self.players_chips[self.player_index])
        elif actionstr == 'fold':
            self.betting_stage = 5
        self.move_next_player()
        self.curbetting_round_actions += 1
        self.cur_chance_node = False

    def is_terminal(self):
        return self.betting_stage > 3

class FishPlayer():  # Do not forget to make parent class as "BasePokerPlayer"

    def __init__(self):
        '''display history and current which action'''
        print('start load')
        self.playsearch = cdll.LoadLibrary('./AlascasiaHoldem.so')
        print('finish load')
        self.ai_id = '0'

    #  we define the logic to make an action through this method. (so this method would be the core of your AI)
    def declare_action(self):
        inaction = bytes(20)
        self.playsearch.getdecision(inaction)
        action = inaction.decode().strip(b'\x00'.decode())
        print('python rec ai action:',action)
        if action not in ['call','fold','allin','check'] and 'raise' not in action:
            print('ai action error:',action)
            raise Exception("ai action error")
        return action  # action returned here is sent to the poker engine

    def receive_game_start_message(self, game_info):
        # print('receive_game_start_message')
        # print(game_info)
        pass

    def receive_round_start_message(self, myid, c1id, c2id):
        '''preflop transmit para to restart'''
        self.playsearch.restart_game(myid ^ 1, c1id, c2id)
        pass

    def receive_street_start_message(self, betting_stage, community_card):
        # print('receive_street_start_message')
        if betting_stage != 0:
            lenc = len(community_card)
            if betting_stage + 2 != lenc:
                raise Exception('betting stage community cards error')

            community_card_idx = bytes(community_card)
            self.playsearch.Next_stage(betting_stage, community_card_idx)
        pass

    def receive_game_update_message(self, actionstr):
        bactionstr = bytes(actionstr, encoding='utf-8')
        self.playsearch.opp_take_action(bactionstr)
        pass

    def receive_round_result_message(self, winners, hand_info, round_state):
        # print('receive_round_result_message')
        # print('winners:', winners)
        # print('hand_info:', hand_info)
        # print('round_state:', round_state)
        pass


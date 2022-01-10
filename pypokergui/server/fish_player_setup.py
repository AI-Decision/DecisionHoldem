import sys
# sys.path.append('.\\pypokerengine')
from pypokerengine.players import BasePokerPlayer
from pypokerengine.engine.card import Card
import math
import pypokerengine.utils.action_utils as AU
class FishPlayer(BasePokerPlayer):  # Do not forget to make parent class as "BasePokerPlayer"

    def __init__(self,_playsearch):
        '''display history and current which action'''
        self.playsearch = _playsearch 
        self.ai_id = '0'

    #  we define the logic to make an action through this method. (so this method would be the core of your AI)
    def declare_action(self, valid_actions, hole_card, round_state):
        inaction = bytes(20)
        self.playsearch.getdecision(inaction)
        action = inaction.decode().strip(b'\x00'.decode())
        print('python rec ai action:',action)
        if action not in ['call','fold','allin','check'] and 'raise' not in action:
            print('ai action error:',action)
            raise Exception("ai action error")
        if 'raise' in action:
            amount = int(action.split(' ')[1])
            return 'raise',amount
        if action == 'check':
            action = 'call'
        return action, 0  # action returned here is sent to the poker engine

    def receive_game_start_message(self, game_info):
        # print('receive_game_start_message')
        # print(game_info)
        pass

    def receive_round_start_message(self, round_count, hole_card, seats):
        '''preflop transmit para to restart'''
        for pl in seats:
            if len(pl['uuid']) <= 2:
                if pl['stack'] == 19900:
                    myid = 1
                else:
                    myid = 0
        hole_card[0] = Card.from_str(hole_card[0])
        hole_card[1] = Card.from_str(hole_card[1])
        c1id = 13 * (int(math.log2(hole_card[0].suit)) - 1) + hole_card[0].rank - 2
        c2id = 13 * (int(math.log2(hole_card[1].suit)) - 1) + hole_card[1].rank - 2
        self.playsearch.restart_game(myid, c1id, c2id)
        pass

    def receive_street_start_message(self, street, round_state):
        # print('receive_street_start_message')
        if street != 'preflop' and round_state['pot']['main']['amount'] != 40000:
            if street == 'flop':
                betting_stage = 1
            elif street == 'turn':
                betting_stage = 2
            elif street == 'river':
                betting_stage = 3
            else:
                raise Exception('betting stage error')
            community_card = round_state['community_card']
            lenc = len(community_card)
            if betting_stage + 2 != lenc:
                raise Exception('betting stage community cards error')
            community_card_idx = []
            for i in range(lenc):
                community_card[i] = Card.from_str(community_card[i])
                community_card_idx.append(
                    13 * (int(math.log2(community_card[i].suit)) - 1) + community_card[i].rank - 2)
            community_card_idx = bytes(community_card_idx)
            self.playsearch.Next_stage(betting_stage, community_card_idx)
        pass

    def receive_game_update_message(self, action, round_state):
        # lgactions = AU.generate_legal_actions(round_state['seats'], round_state['next_player'], round_state['small_blind_amount'])
        if action['player_uuid'] == self.ai_id:
            return
        actionstr = action['action']
        for pl in round_state['seats']:
            if pl['uuid'] == action['player_uuid']:
                if action["action"] == "raise" and pl['stack'] == 0:
                    actionstr = 'allin'
        if actionstr != 'allin' and action["action"] == "raise":
            actionstr = 'raise ' + str(action['amount'])
        bactionstr = bytes(actionstr, encoding='utf-8')
        self.playsearch.opp_take_action(bactionstr)
        pass

    def receive_round_result_message(self, winners, hand_info, round_state):
        # print('receive_round_result_message')
        # print('winners:', winners)
        # print('hand_info:', hand_info)
        # print('round_state:', round_state)
        pass


def setup_ai():
    return FishPlayer()

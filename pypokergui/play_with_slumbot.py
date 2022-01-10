import requests
import os
import sys
import warnings
from fish_player_setup import FishPlayer, State
import re

host = 'slumbot.com'

NUM_STREETS = 4
SMALL_BLIND = 50
BIG_BLIND = 100
STACK_SIZE = 20000
bot = FishPlayer()
cards_dic = {
     '2s':0,
     '3s':1,
     '4s':2,
     '5s':3,
     '6s':4,
     '7s':5,
     '8s':6,
     '9s':7,
     'Ts':8,
     'Js':9,
     'Qs':10,
     'Ks':11,
     'As':12,
     '2c':13,
     '3c':14,
     '4c':15,
     '5c':16,
     '6c':17,
     '7c':18,
     '8c':19,
     '9c':20,
     'Tc':21,
     'Jc':22,
     'Qc':23,
     'Kc':24,
     'Ac':25,
     '2d':26,
     '3d':27,
     '4d':28,
     '5d':29,
     '6d':30,
     '7d':31,
     '8d':32,
     '9d':33,
     'Td':34,
     'Jd':35,
     'Qd':36,
     'Kd':37,
     'Ad':38,
     '2h':39,
     '3h':40,
     '4h':41,
     '5h':42,
     '6h':43,
     '7h':44,
     '8h':45,
     '9h':46,
     'Th':47,
     'Jh':48,
     'Qh':49,
     'Kh':50,
     'Ah':51
}


def Login(username, password):
    data = {"username": username, "password": password}
    response = requests.post(f'https://{host}/api/login', json=data)
    success = getattr(response, 'status_code') == 200
    if not success:
        print('Status code: %s' % repr(response.status_code))
        try:
            print('Error response: %s' % repr(response.json()))
        except ValueError:
            pass
        sys.exit(-1)

    try:
        r = response.json()
    except ValueError:
        print('Could not get JSON from response')
        sys.exit(-1)

    if 'error_msg' in r:
        print('Error: %s' % r['error_msg'])
        sys.exit(-1)

    token = r.get('token')
    if not token:
        print('Did not get token in response to /api/login')
        sys.exit(-1)
    return token
def ParseAction(action):
    """
    Returns a dict with information about the action passed in.
    Returns a key "error" if there was a problem parsing the action.
    pos is returned as -1 if the hand is over; otherwise the position of the player next to act.
    street_last_bet_to only counts chips bet on this street, total_last_bet_to counts all
      chips put into the pot.
    Handles action with or without a final '/'; e.g., "ck" or "ck/".
    """
    st = 0
    street_last_bet_to = BIG_BLIND
    total_last_bet_to = BIG_BLIND
    last_bet_size = BIG_BLIND - SMALL_BLIND
    last_bettor = 0
    sz = len(action)
    pos = 1
    if sz == 0:
        return {
            'st': st,
            'pos': pos,
            'street_last_bet_to': street_last_bet_to,
            'total_last_bet_to': total_last_bet_to,
            'last_bet_size': last_bet_size,
            'last_bettor': last_bettor,
        }

    check_or_call_ends_street = False
    i = 0
    while i < sz:
        if st >= NUM_STREETS:
            return {'error': 'Unexpected error'}
        c = action[i]
        i += 1
        if c == 'k':
            if last_bet_size > 0:
                return {'error': 'Illegal check'}
            if check_or_call_ends_street:
	        # After a check that ends a pre-river street, expect either a '/' or end of string.
                if st < NUM_STREETS - 1 and i < sz:
                    if action[i] != '/':
                        return {'error': 'Missing slash'}
                    i += 1
                if st == NUM_STREETS - 1:
	            # Reached showdown
                    pos = -1
                else:
                    pos = 0
                    st += 1
                street_last_bet_to = 0
                check_or_call_ends_street = False
            else:
                pos = (pos + 1) % 2
                check_or_call_ends_street = True
        elif c == 'c':
            if last_bet_size == 0:
                return {'error': 'Illegal call'}
            if total_last_bet_to == STACK_SIZE:
	        # Call of an all-in bet
	        # Either allow no slashes, or slashes terminating all streets prior to the river.
                if i != sz:
                    for st1 in range(st, NUM_STREETS - 1):
                        if i == sz:
                            return {'error': 'Missing slash (end of string)'}
                        else:
                            c = action[i]
                            i += 1
                            if c != '/':
                                return {'error': 'Missing slash'}
                if i != sz:
                    return {'error': 'Extra characters at end of action'}
                st = NUM_STREETS - 1
                pos = -1
                last_bet_size = 0
                return {
                    'st': st,
                    'pos': pos,
                    'street_last_bet_to': street_last_bet_to,
                    'total_last_bet_to': total_last_bet_to,
                    'last_bet_size': last_bet_size,
                    'last_bettor': last_bettor,
                }
            if check_or_call_ends_street:
	        # After a call that ends a pre-river street, expect either a '/' or end of string.
                if st < NUM_STREETS - 1 and i < sz:
                    if action[i] != '/':
                        return {'error': 'Missing slash'}
                    i += 1
                if st == NUM_STREETS - 1:
	            # Reached showdown
                    pos = -1
                else:
                    pos = 0
                    st += 1
                street_last_bet_to = 0
                check_or_call_ends_street = False
            else:
                pos = (pos + 1) % 2
                check_or_call_ends_street = True
            last_bet_size = 0
            last_bettor = -1
        elif c == 'f':
            if last_bet_size == 0:
                return {'error', 'Illegal fold'}
            if i != sz:
                return {'error': 'Extra characters at end of action'}
            pos = -1
            return {
                'st': st,
                'pos': pos,
                'street_last_bet_to': street_last_bet_to,
                'total_last_bet_to': total_last_bet_to,
                'last_bet_size': last_bet_size,
                'last_bettor': last_bettor,
            }
        elif c == 'b':
            j = i
            while i < sz and action[i] >= '0' and action[i] <= '9':
                i += 1
            if i == j:
                return {'error': 'Missing bet size'}
            try:
                new_street_last_bet_to = int(action[j:i])
            except (TypeError, ValueError):
                return {'error': 'Bet size not an integer'}
            new_last_bet_size = new_street_last_bet_to - street_last_bet_to
            # Validate that the bet is legal
            remaining = STACK_SIZE - street_last_bet_to
            if last_bet_size > 0:
                min_bet_size = last_bet_size
	        # Make sure minimum opening bet is the size of the big blind.
                if min_bet_size < BIG_BLIND:
                    min_bet_size = BIG_BLIND
            else:
                min_bet_size = BIG_BLIND
            # Can always go all-in
            if min_bet_size > remaining:
                min_bet_size = remaining
            # if new_last_bet_size < min_bet_size:
            #     return {'error': 'Bet too small'}
            max_bet_size = remaining
            if new_last_bet_size > max_bet_size:
                return {'error': 'Bet too big'}
            last_bet_size = new_last_bet_size
            street_last_bet_to = new_street_last_bet_to
            total_last_bet_to += last_bet_size
            last_bettor = pos
            pos = (pos + 1) % 2
            check_or_call_ends_street = True
        else:
            return {'error': 'Unexpected character in action'}

    return {
        'st': st,
        'pos': pos,
        'street_last_bet_to': street_last_bet_to,
        'total_last_bet_to': total_last_bet_to,
        'last_bet_size': last_bet_size,
        'last_bettor': last_bettor,
    }

def NewHand(token):
    data = {}
    if token:
        data['token'] = token
    response = requests.post(f'https://{host}/api/new_hand', headers={}, json=data)
    success = getattr(response, 'status_code') == 200
    if not success:
        print('Status code: %s' % repr(response.status_code))
        raise ValueError('Status error')
        # try:
        #     print('Error response: %s' % repr(response.json()))
        # except ValueError:
        #     pass
        # sys.exit(-1)

    try:
        r = response.json()
    except ValueError:
        print('Could not get JSON from response')
        raise ValueError('json error')
        # sys.exit(-1)

    if 'error_msg' in r:
        print('Error: %s' % r['error_msg'])
        raise ValueError('msg error')
        # sys.exit(-1)

    return r

def Act(token, action):
    data = {'token': token, 'incr': action}
    response = requests.post(f'https://{host}/api/act', headers={}, json=data)
    success = getattr(response, 'status_code') == 200
    if not success:
        print('Status code: %s' % repr(response.status_code))
        raise ValueError('Status error')
        # try:
        #     print('Error response: %s' % repr(response.json()))
        # except ValueError:
        #     pass
        # sys.exit(-1)

    try:
        r = response.json()
    except ValueError:
        print('Could not get JSON from response')
        raise ValueError('json error')

    if 'error_msg' in r:
        print('Error: %s' % r['error_msg'])
        raise ValueError('msg error')

    return r
def change_action_sendstr(action,state):
    if action == "fold":
        incr = 'f'
    elif action == "call":
        incr = 'c'
    elif action == "check":
        incr = 'k'
    elif action == "allin":
        incr = 'b'+ str(state.last_round_bets)
        # if state.players_chips[state.player_index] != state.last_round_bets:
        #     raise Exception('error allin',state.last_round_bets,state.players_chips[state.player_index])
    else:
        incr = "b" + str(int(action.split(' ')[1]))
    return incr
def change_action_recvstr(actionstr,state):
    if actionstr == "f":
        incr = 'fold'
    elif actionstr == "c":
        incr = 'call'
    elif actionstr == "k":
        incr = 'check'
    elif actionstr[0] == "b":
        if state.last_round_bets == int(actionstr[1:]):
            incr = 'allin'
        else:
            incr = "raise " + actionstr[1:]
    else:
        raise Exception('error recv actionstr',actionstr)
    return incr
def PlayHand(token):
    r = NewHand(token)
    # client_pos = r.get('client_pos')
    # hole_cards = r.get('hole_cards')
    pattern = r"c|k|f|b\d+"
    new_token = r.get('token')
    state = State()
    print('New token: %s' % token)
    if new_token:
        token = new_token
    hole_card = None
    while True:
        print('-----------------')
        print(repr(r))
        action = r.get('action')
        client_pos = r.get('client_pos')
        hole_cards = r.get('hole_cards')
        if (client_pos == 1 and len(action) == 0) or (client_pos == 0 and len(re.findall(pattern,action.split('/')[0])) == 1):
            print('Client pos: %i' % client_pos)

            hole_card = [cards_dic[hole_cards[0]],cards_dic[hole_cards[1]]]
            print('Client hole cards: %s' % repr(hole_cards))
            """
            client_pos = 1 是小盲注
            client_pos = 0 是大盲注
            """
            bot.receive_round_start_message(client_pos,hole_card[0],hole_card[1])
        if hole_card is None:
            raise Exception('init cards error')
        board = r.get('board')
        winnings = r.get('winnings')
        print('Action: %s' % action)
        if winnings is not None:
            if state.is_terminal() == False:
                a = ParseAction(action)
                last_actions = action.split("/")
                last_actions = last_actions[state.betting_stage]
                list_last_actions = re.findall(pattern, last_actions)
                if a.get("st") != state.betting_stage and a.get("st") != 3:
                    raise Exception('error allin terminal game', state.betting_stage)
                for i in range(state.curbetting_round_actions, len(list_last_actions)):
                    incr = change_action_recvstr(list_last_actions[state.curbetting_round_actions],state)
                    bot.receive_game_update_message(incr)
                    state.apply_action(incr)
            if state.is_terminal() == False:
                raise Exception('error terminal game',state.betting_stage)
            print('Hand winnings: %i' % winnings)
            return (token, winnings)

        a = ParseAction(action)
        print(a)
        if 'error' in a:
            print('Error parsing action %s: %s' % (action, a['error']))
            sys.exit(-1)

        if state.cur_chance_node == True or state.betting_stage != a.get("st") or a['last_bettor'] == -1: # 刚刚发出公共牌
            if state.betting_stage != a.get("st") and a.get("st") - state.betting_stage != 1:
                raise Exception('error next round')
            board_idx = []
            for card in board:
                board_idx.append(cards_dic[card])
            print('Board: %s' % repr(board))
            if state.cur_chance_node == False:
                if a.get("st") <= 0:
                    raise Exception("error chance node1")
                last_actions = action.split("/")
                last_actions = last_actions[a.get("st") - 1]
                list_last_actions = re.findall(pattern, last_actions)
                for i in range(state.curbetting_round_actions, len(list_last_actions)):
                    incr = change_action_recvstr(list_last_actions[state.curbetting_round_actions],state)
                    bot.receive_game_update_message(incr)
                    state.apply_action(incr)
            if state.cur_chance_node == False:
                raise Exception("error chance node+++++++++++")
            bot.receive_street_start_message(state.betting_stage,board_idx)

        # 接收对手动作，并做新动作
        actions = action.split("/")
        cur_actions = actions[a.get("st")]
        """
        TODO:
        依据通信协议解析对手动作opponent_action
        动作序列例子：'ck/kk/b100c/b200'
        ‘/’负责分开street


        """
        list_cur_actions = re.findall(pattern,cur_actions)
        for i in range(state.curbetting_round_actions,len(list_cur_actions)):
            incr = change_action_recvstr(list_cur_actions[state.curbetting_round_actions],state)
            bot.receive_game_update_message(incr)
            state.apply_action(incr)
        if client_pos != state.player_index:
            raise Exception('error seq to take action')
        #AI执行动作
        incr = bot.declare_action()
        actionstr = change_action_sendstr(incr,state)
        if a['last_bet_size'] == 0 and actionstr == 'c':
            actionstr = 'k'
        state.apply_action(incr)
        """
        TODO:
        根据CASIA_Poker的通信规则解析动作
        if action == "fold":
            incr = 'f'
        elif action == "call":
            incr = 'c'
        elif action == "check":
            incr = 'k'
        elif action == "allin":
            incr = 'b20000'
        else:
            incr = "b???"
        最后返回给slumbot服务器的动作为incr，是一个字符串
        """
        r = Act(token, actionstr)

import time
def main():
    username = 'zqbDec'
    password = 'zqbDec@2021'
    token = Login(username, password)
    total_winnings = 0
    play_times = 0
    while True:
        try:
            (token, hand_winnings) = PlayHand(token)
            total_winnings += hand_winnings
            play_times += 1
            print('play times: %d, total Winnings: %i' % (play_times,total_winnings))
        except ValueError as e:
            print(e)
            time.sleep(1)
        except Exception as e:
            print('未知异常',e)



if __name__ == '__main__':
    main()

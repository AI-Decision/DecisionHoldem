import sys
import json
import struct
import socket
import random
from fish_player_setup import FishPlayer, State

server_ip = "holdem.ia.ac.cn"
server_port = 18888                    
room_id = int(sys.argv[1])
room_number = int(sys.argv[2])
name = sys.argv[3]
game_number = int(sys.argv[4])
bots = sys.argv[5:] if len(sys.argv) > 5 else []

"""
与智能体的对战指令:
python play_against_ia_v4.py 888891 2 Bot 1 OpenStackTwo
python play_against_ia_v4.py 888892 2 Bot 1 OpenStack
python play_against_ia_v4.py 888893 2 Bot 1 LiShuokai
python play_against_ia_v4.py 888894 2 Bot 1 YangJun
python play_against_ia_v4.py 888895 2 Bot 1 YuanWeilin
python play_against_ia_v4.py 888896 2 Bot 1 QianTao
"""

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

def get_action(data):
    if 'fold' in data['legal_actions']:
        data['legal_actions'].remove('fold')
    choose_index = random.randint(0, len(data['legal_actions']) - 1)
    if data['legal_actions'][choose_index] == 'raise':
        action = 'r' + str(random.randint(data['raise_range'][0], data['raise_range'][1]))
    else:
        action = data['legal_actions'][choose_index]
    return action


def sendJson(request, jsonData):
    data = json.dumps(jsonData).encode()
    request.send(struct.pack('i', len(data)))
    request.sendall(data)


def recvJson(request):
    data = request.recv(4)
    length = struct.unpack('i', data)[0]
    data = request.recv(length).decode()
    while len(data) != length:
        data = data + request.recv(length - len(data)).decode()
    data = json.loads(data)
    return data

def transpose(action_history):
    def f(x):
        if x == "call":
            return "c"
        if x == "check":
            return "c"
        if x == "fold":
            return "f"
        return x
    game_all = []
    for one_round in action_history:
        round_all = []
        for one_action in one_round:
            action = str(one_action["position"]) + ":" + f(str(one_action["action"]))
            round_all.append(action)
        round_all = ','.join(round_all)
        game_all.append(round_all)
    game_all = '/'.join(game_all)
    return game_all
def change_action_sendstr(action,state):
    if action == "allin":
        incr = 'r'+ str(state.last_round_bets)
        # if state.players_chips[state.player_index] != 0:
        #     raise Exception('error allin')
    elif action[:5] == 'raise':
        incr = "r" + str(int(action.split(' ')[1]))
    elif action not in ['call','fold','check']:
        raise Exception('error send action')
    else:
        incr = action
    return incr
def change_action_recvstr(actionstr,state):
    if actionstr[0] == "r":
        if (state.last_round_bets) == int(actionstr[1:]):
            incr = 'allin'
        elif (state.last_round_bets) < int(actionstr[1:]):
            raise Exception('error recv raise',actionstr)
        else:
            incr = "raise " + actionstr[1:]
    elif actionstr not in ['call','fold','check']:
        raise Exception('error recv actionstr',actionstr)
    else:
        incr = actionstr
    return incr
if __name__ == "__main__":
    bot = FishPlayer()
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((server_ip, server_port))
    message = dict(info='connect', room_id=room_id, name=name, room_number=room_number, bots=bots, game_number=game_number)
    sendJson(client, message)
    state = State()
    total_win = 0
    game_count = 0
    client_pos = -1
    while True:

        data = recvJson(client)
        if data['info'] == 'state':
            if data['action_history'][0] == []:
                if state.betting_stage != 0 or state.curbetting_round_actions != 0:
                    raise Exception('error preflop dealing')
                client_pos = int(data['position']) ^ 1
                private_card = ''.join(data['private_card'])
                card1 = private_card[0:2]
                card2 = private_card[2:4]
                hole_card = [cards_dic[card1],cards_dic[card2]]
                """
                client_pos = 1 是小盲注
                client_pos = 0 是大盲注
                """
                bot.receive_round_start_message(client_pos,hole_card[0],hole_card[1])
            cur_betting_state = len(data['public_card'])
            if cur_betting_state > 0:
                cur_betting_state = cur_betting_state - 2;
            if state.cur_chance_node == True or cur_betting_state > state.betting_stage:
                list_cur_actions = data['action_history'][state.betting_stage]
                for i in range(state.curbetting_round_actions, len(list_cur_actions)):
                    incr = change_action_recvstr(list_cur_actions[state.curbetting_round_actions]['action'], state)
                    bot.receive_game_update_message(incr)
                    state.apply_action(incr)
                if state.cur_chance_node == False:
                    raise Exception("error chance node+++++++++++")
                state.cur_chance_node = False
                board = data['public_card']
                board_idx = []
                for card in board:
                    board_idx.append(cards_dic[card])
                bot.receive_street_start_message(state.betting_stage,board_idx)
            position = data['position']
            opponent_position = 1 - position
            bot_bet = data['players'][position]['total_money'] - data['players'][position]['money_left']
            opponent_bet = data['players'][opponent_position]['total_money'] - data['players'][opponent_position]['money_left']
            pot = bot_bet + opponent_bet

            if client_pos ^ 1 == data['action_position']: # and state.player_index == client_pos ^ 1
                list_cur_actions = data['action_history'][state.betting_stage]
                for i in range(state.curbetting_round_actions, len(list_cur_actions)):
                    incr = change_action_recvstr(list_cur_actions[state.curbetting_round_actions]['action'], state)
                    bot.receive_game_update_message(incr)
                    state.apply_action(incr)
                if client_pos != state.player_index:
                    raise Exception('error seq to take action')
                incr = bot.declare_action()
                actionstr = change_action_sendstr(incr,state)
                if actionstr == "call":
                    if bot_bet == opponent_bet:
                        actionstr = 'check'

                """
                根据通信协议对action进行解析
                if action['action'] == "fold":
                    action_send = 'fold'

                elif action['action'] == "call":
                    if bot_bet == opponent_bet:
                        action_send = 'check'
                    else:
                        action_send = 'call'

                elif action['action'] == "allin":
                    action_send = 'r' + str(data['raise_range'][1])

                elif action['action'] == "raise":
                  pass
                else:
                    action_send = 'fold'  
                """
                sendJson(client, {'action': actionstr, 'info': 'action'})
                state.apply_action(incr)

        elif data['info'] == 'result':
            state = State()
            game_count += 1
            total_win = total_win + data['players'][position]['win_money']
            print("-------------- game: {} -----------------".format(game_count))
            print("win_money: {}".format(data['players'][position]['win_money']))
            print("players: {}".format(', '.join([player['name'] for player in data['players']])))
            print("hand: {}".format(', '.join([''.join(card) for card in data['player_card']])))
            print("board: {}".format(''.join(data['public_card'])))
            print("action_history: {}".format(transpose(data['action_history'])))
            print("total_win: {}".format(total_win))
            sendJson(client, {'info': 'ready', 'status': 'start'})
        else:
            print("-------------- finish game ---------------")
            print(data)
            break
    client.close()

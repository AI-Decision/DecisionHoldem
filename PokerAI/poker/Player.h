#pragma once
#include "Card.h"
#include <assert.h>
#include <exception>
#include <iostream>
using namespace std;
class Player
{
public:
	/****************************   40000 > max short  change unsigned****************************************************/
	unsigned short initial_chips, n_chips;
	unsigned char preflopcluster;
	bool active;
	unsigned* clusters = new unsigned[4];
	Player() {}
	Player(int initial_chips) {
		this->initial_chips = initial_chips;
		this->n_chips = initial_chips;
		this->active = true;
	}
	void reset() {
		this->n_chips = this->initial_chips;
		this->active = true;
	}
	void add_chips(int chips) {
		this->n_chips += chips;
	}
	void call(int call_bet) {
		add_to_pot(call_bet);
	}
	void raise_to(int chips) {
		add_to_pot(chips);
	}
	void add_to_pot(int chips) {
		n_chips -= chips;
		assert(n_chips >= 0);
	}
	inline int n_bet_chips() {
		return initial_chips - n_chips;
	}
	~Player() {
		//cout << "del player" << id << endl;
	}
};

class SearchPlayer
{
public:
	unsigned short initial_chips, n_chips;
	bool active;
	SearchPlayer(int initial_chips = 20000) {
		this->initial_chips = initial_chips;
		this->n_chips = initial_chips;
		this->active = true;
	}
	void reset() {
		this->n_chips = this->initial_chips;
		this->active = true;
	}
	void add_chips(int chips) {
		this->n_chips += chips;
	}
	void call(int call_bet) {
		add_to_pot(call_bet);
	}
	void raise_to(int chips) {
		add_to_pot(chips);
	}
	void add_to_pot(int chips) {
		n_chips -= chips;
		if (n_chips < 0) 
			throw exception();
		
	}
	inline int n_bet_chips() {
		return initial_chips - n_chips;
	}
};

#pragma once

#include "state.hpp"

#include <cstdint>
#include <cmath>


float unit_count(uint8_t ranks_left) {
	float value = 0;
	if (ranks_left & 2) value += 1.0f;   // spy
	if (ranks_left & 4) value += 1.0f;   // scout
	if (ranks_left & 8) value += 1.0f;   // scout
	if (ranks_left & 16) value += 1.0f;  // miner
	if (ranks_left & 32) value += 1.0f;  // general
	if (ranks_left & 64) value += 1.0f;  // marshal
	if (ranks_left & 128) value += 1.0f; // bomb
	return value;
}


int distance_to_flag(State *s, int me) {
	int offset = (1 - me) * 8, flag_pos;
	for (int pos = 0; pos < 100; pos++) {
		if (s->board[pos] < 16 && s->board[pos] / 8 == 1 - me && (s->pieces[s->board[pos]] & 7) == RANK_FLAG) {
			flag_pos = pos;
			break;
		}
	}

	int flag_x = flag_pos % 10, flag_y = flag_pos / 10, min_dis = 1000;
	int pos = 0;
	for (int y = 0; y < 10; y++) {
		for (int x = 0; x < 10; x++) {
			if (s->board[pos] < 16 && s->board[pos] / 8 == me) {
				int dis = abs(flag_x - x) + abs(flag_y - y);
				if (dis < min_dis) {
					min_dis = dis;
				}
			}
			pos++;
		}
	}
	
	return min_dis;
}


float transform(float value, double p) {
	return 1.0f - (float)pow(1.0f - value, p);
}


float simulate(State *s, int me, double p, float c) {
	float value = (unit_count(s->ranks_left[me]) / unit_count(s->ranks_left[1 - me])) / 7.0f;
	value += c / distance_to_flag(s, me);
	value /= (1.0f + c);
	return transform(value, p);
}

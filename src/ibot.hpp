#pragma once

#include <cstdint>
#include <string>
#include <cassert>
#include <random>

#include "state.hpp"

#define MOVE_BIT 8
#define REVEAL_BIT 16


struct StartPosition {
	uint8_t position[8];
};


class Bot {
public:
	uint8_t me;
	State state;
	uint8_t start_position[8];

	virtual std::string identifier() = 0;
	virtual uint16_t play_move() = 0;

	StartPosition prepare(uint8_t me) {
		this->me = me;

		StartPosition start_position;
		for (int i = 0; i < 8; i++) {
			start_position.position[i] = me ? mirror_position(this->start_position[i]) : this->start_position[i];
		}

		return start_position;
	}

	uint8_t mirror_position(uint8_t pos) {
		uint8_t y = pos / 10;
		uint8_t x = pos % 10;
		return 99 - 10 * y - x;
	}

	void set_board(State state) {
		this->state = state;
	}

	void reveal(uint8_t idx, uint8_t rank) {
		state.reveal(idx, rank);
	}

	void reveal_at(uint8_t pos, uint8_t rank) {
		uint8_t idx = state.board[pos];
		assert(idx < 16);
		state.reveal(idx, rank);
	}

	uint8_t do_move(uint16_t move) {
		uint8_t from = move >> 8, to = move & 255;
		uint8_t from_x = from % 10, from_y = from / 10, to_x = to % 10, to_y = to / 10;

		if (abs(from_x - to_x) + abs(from_y - to_y) > 1) {
			reveal_at(from, RANK_SCOUT);
			state.pieces[state.board[from]] |= REVEAL_BIT;
		}

		if (state.board[to] != CELL_EMPTY) {
			state.pieces[state.board[from]] |= REVEAL_BIT;
			state.pieces[state.board[to]] |= REVEAL_BIT;
		}

		state.pieces[state.board[from]] |= MOVE_BIT;
		return state.do_move(move);
	}

	uint16_t moves[50];
	int enumerate_moves(State *s) {
		int counter = 0;
		int active_player = s->active_player * 8;

		for (int y = 0, i = 0; y < 10; y++) {
			for (int x = 0; x < 10; x++, i++) {
				uint16_t base = (i << 8) + i;
				if (s->board[i] < 16 && (s->board[i] & 8) == active_player) {
					uint8_t rank = s->pieces[s->board[i]] & 7;
					if (rank != RANK_FLAG && rank != RANK_BOMB) {
						if (rank == RANK_SCOUT) {
							if (x > 0) {
								int xx = x, ii = base, to = i;
								while (true) {
									xx--; ii--; to--;
									if (xx < 0 || s->board[to] == CELL_WATER) break;

									assert(s->board[to] < 16 || s->board[to] == CELL_EMPTY);
									if (s->board[to] < 16) {
										if ((s->board[to] & 8) != active_player) { // bug here
											assert(s->board[ii % 256] / 8 != s->board[ii / 256] / 8);
											moves[counter++] = ii;
										}
										break;
									}
									assert(s->board[ii % 256] / 8 != s->board[ii / 256] / 8);
									moves[counter++] = ii;
								}
							}
							if (x < 9) {
								int xx = x, ii = base, to = i;
								while (true) {
									xx++; ii++; to++;
									if (xx >= 10 || s->board[to] == CELL_WATER) break;

									assert(s->board[to] < 16 || s->board[to] == CELL_EMPTY);
									if (s->board[to] < 16) {
										if ((s->board[to] & 8) != active_player) {
											assert(s->board[ii % 256] / 8 != s->board[ii / 256] / 8);
											moves[counter++] = ii;
										}
										break;
									}
									assert(s->board[ii % 256] / 8 != s->board[ii / 256] / 8);
									moves[counter++] = ii;
								}
							}
							if (y > 0) {
								int yy = y, ii = base, to = i;
								while (true) {
									yy--; ii -= 10; to -= 10;
									if (yy < 0 || s->board[to] == CELL_WATER) break;

									assert(s->board[to] < 16 || s->board[to] == CELL_EMPTY);
									if (s->board[to] < 16) {
										if ((s->board[to] & 8) != active_player) {
											assert(s->board[ii % 256] / 8 != s->board[ii / 256] / 8);
											moves[counter++] = ii;
										}
										break;
									}
									assert(s->board[ii % 256] / 8 != s->board[ii / 256] / 8);
									moves[counter++] = ii;
								}
							}
							if (y < 9) {
								int yy = y, ii = base, to = i;
								while (true) {
									yy++; ii += 10; to += 10;
									if (yy >= 10 || s->board[to] == CELL_WATER) break;

									assert(s->board[to] < 16 || s->board[to] == CELL_EMPTY);
									if (s->board[to] < 16) {
										if ((s->board[to] & 8) != active_player) {
											assert(s->board[ii % 256] / 8 != s->board[ii / 256] / 8);
											moves[counter++] = ii;
										}
										break;
									}
									assert(s->board[ii % 256] / 8 != s->board[ii / 256] / 8);
									moves[counter++] = ii;
								}
							}
						}
						else {
							if (x > 0 && (s->board[i - 1] == CELL_EMPTY || (s->board[i - 1] < 16 && (s->board[i - 1] & 8) != active_player))) {
								moves[counter++] = base - 1;
							}
							if (x < 9 && (s->board[i + 1] == CELL_EMPTY || (s->board[i + 1] < 16 && (s->board[i + 1] & 8) != active_player))) {
								moves[counter++] = base + 1;
							}
							if (y > 0 && (s->board[i - 10] == CELL_EMPTY || (s->board[i - 10] < 16 && (s->board[i - 10] & 8) != active_player))) {
								moves[counter++] = base - 10;
							}
							if (y < 9 && (s->board[i + 10] == CELL_EMPTY || (s->board[i + 10] < 16 && (s->board[i + 10] & 8) != active_player))) {
								moves[counter++] = base + 10;
							}
						}
					}
				}
			}
		}

		return counter;
	}
};

#pragma once

#include <iostream>
#include <cassert>
#include <cstdint>


#define DEAD 0xFF

#define CELL_EMPTY      0xFF
#define CELL_WATER      0xFE

#define RANK_FLAG    0
#define RANK_SPY     1
#define RANK_SCOUT   2
#define RANK_MINER   3
#define RANK_GENERAL 4
#define RANK_MARSHAL 5
#define RANK_BOMB    6
#define RANK_UNKNOWN 7

#define STARTING_PLAYER 0

#define MOVE_RESULT_NORMAL 0
#define MOVE_RESULT_DRAW   1
#define MOVE_RESULT_P0_WIN 2
#define MOVE_RESULT_P1_WIN 3

#define BATTLE_RESULT_ATTACKER 0
#define BATTLE_RESULT_DEFENDER 1
#define BATTLE_RESULT_DRAW 2


struct State {
	uint8_t active_player = 0;
	uint16_t turn = 0;
	uint8_t board[100] = { CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY, CELL_EMPTY };
	uint8_t pieces[16] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	uint8_t ranks_left[2] = { 0xff, 0xff };


	void reveal(uint8_t idx, uint8_t rank) {
		assert(idx < 16);
		assert(rank < 7);

		uint8_t current_rank = pieces[idx] & 7;
		assert(current_rank == RANK_UNKNOWN || current_rank == rank);

		pieces[idx] = (pieces[idx] & 0xf8) | rank;
	}


	void kill(uint8_t idx) {
		assert(idx < 16);
		uint8_t owner = idx / 8;
		uint8_t rank = pieces[idx] & 7;
		uint8_t rank_idx = rank + (rank > 2);

		if (rank_idx == 2 && !(ranks_left[owner] & 4)) {
			rank_idx++;
		}

		uint8_t bit = 1 << rank_idx;
		assert(ranks_left[owner] & bit);

		uint8_t mask = ~bit;
		ranks_left[owner] &= mask;
	}

	char to_char(uint8_t cell) {
		if (cell == CELL_EMPTY) return ' ';
		if (cell == CELL_WATER) return '~';
		if (cell < 16) {
			uint8_t owner = cell / 8;
			uint8_t rank = pieces[cell] & 7;
			if (rank == RANK_UNKNOWN) {
				return '?';
			}
			if (owner == 0) {
				return 'a' + (pieces[cell] & 7);
			}
			else {
				return 'A' + (pieces[cell] & 7);
			}
		}
		return '!';
	}

	void draw() {
		std::cout << "  0123456789\n";
		for (int y = 0; y < 10; y++) {
			std::cout << y << " ";
			for (int x = 0; x < 10; x++) {
				std::cout << to_char(board[y * 10 + x]);
			}
			std::cout << '\n';
		}
	}


	bool is_piece_stuck_for(uint8_t player) {
		player *= 8;
		for (int y = 0, pos = 0; y < 10; y++) {
			for (int x = 0; x < 10; x++, pos++) {
				if ((board[pos] & 8) == player) {
					if (x > 0 && (board[pos - 1] & 8) != player) {
						return false;
					}
					if (x < 9 && (board[pos + 1] & 8) != player) {
						return false;
					}
					if (y > 0 && (board[pos - 10] & 8) != player) {
						return false;
					}
					if (y < 9 && (board[pos + 10] & 8) != player) {
						return false;
					}
					return true;
				}
			}
		}
		return false;
	}

	uint8_t battle(uint8_t attacker_rank, uint8_t defender_rank) {
		assert(attacker_rank != RANK_UNKNOWN && defender_rank != RANK_UNKNOWN);
		assert(attacker_rank == RANK_SPY || attacker_rank == RANK_SCOUT || attacker_rank == RANK_MINER || attacker_rank == RANK_GENERAL || attacker_rank == RANK_MARSHAL);

		if (attacker_rank == defender_rank) {
			return BATTLE_RESULT_DRAW;
		}

		if ((attacker_rank == RANK_MINER && defender_rank == RANK_BOMB) || (attacker_rank == RANK_SPY && defender_rank == RANK_MARSHAL)) {
			return BATTLE_RESULT_ATTACKER;
		}

		return attacker_rank > defender_rank ? BATTLE_RESULT_ATTACKER : BATTLE_RESULT_DEFENDER;
	}

	uint8_t do_move(uint16_t move) {
		uint8_t from = move >> 8, to = move & 255;
		assert(board[from] < 16);
		assert(board[from] / 8 == active_player);

		uint8_t attacker_rank = pieces[board[from]] & 7;
		assert(attacker_rank == RANK_SPY || attacker_rank == RANK_SCOUT || attacker_rank == RANK_MINER || attacker_rank == RANK_GENERAL || attacker_rank == RANK_MARSHAL || attacker_rank == RANK_UNKNOWN);

		uint8_t from_x = from % 10, from_y = from / 10, to_x = to % 10, to_y = to / 10;
		assert(abs(from_x - to_x) + abs(from_y - to_y) == 1 || attacker_rank == RANK_SCOUT);

		bool was_battle = false;
		if (board[to] < 16) {
			was_battle = true;
			assert(board[to] < 16);
			assert(board[to] / 8 == 1 - active_player);

			uint8_t defender_rank = pieces[board[to]] & 7;
			uint8_t battle_result = battle(attacker_rank, defender_rank);
			
			if (battle_result == BATTLE_RESULT_ATTACKER) {
				kill(board[to]);
			}
			if (battle_result == BATTLE_RESULT_DEFENDER) {
				kill(board[from]);
				board[from] = board[to];
			}
			else if (battle_result == BATTLE_RESULT_DRAW) {
				kill(board[from]);
				kill(board[to]);
				board[from] = CELL_EMPTY;
			}
		}

		board[to] = board[from];
		board[from] = CELL_EMPTY;

		bool p0_dead = false, p1_dead = false;

		if (((ranks_left[0] & 1) == 0) || ((ranks_left[0] & 0x7e) == 0)) {
			p0_dead = true;
		}
		if (((ranks_left[1] & 1) == 0) || ((ranks_left[1] & 0x7e) == 0)) {
			p1_dead = true;
		}

		if (was_battle) {
			uint8_t moving_ranks_0 = ranks_left[0] & 0x7e;
			if (!p0_dead && (moving_ranks_0 & (moving_ranks_0 - 1)) == 0) {
				p0_dead &= !is_piece_stuck_for(0);
			}

			uint8_t moving_ranks_1 = ranks_left[1] & 0x7e;
			if (!p1_dead && (moving_ranks_1 & (moving_ranks_1 - 1)) == 0) {
				p1_dead &= !is_piece_stuck_for(1);
			}
		}

		active_player = 1 - active_player;

		if (p0_dead && p1_dead) {
			return MOVE_RESULT_DRAW;
		}

		if (p0_dead) {
			return MOVE_RESULT_P1_WIN;
		}

		if (p1_dead) {
			return MOVE_RESULT_P0_WIN;
		}

		return MOVE_RESULT_NORMAL;
	}
};

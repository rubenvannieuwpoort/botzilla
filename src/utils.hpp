#pragma once

#include "ibot.hpp"

#include <time.h>
#include <random>
#include <thread>


#if defined (_MSC_VER)  // Visual studio
    #define thread_local __declspec( thread )
#elif defined (__GCC__) // GCC
    #define thread_local __thread
#endif


// these should be kept in sync with MOVE_RESULT !!!
#define GAME_RESULT_DRAW 1
#define GAME_RESULT_BOT0 2
#define GAME_RESULT_BOT1 3


thread_local std::mt19937 *gen = nullptr;

void init_rand() {
	std::hash<std::thread::id> hasher;
	gen = new std::mt19937(clock() + hasher(std::this_thread::get_id()));
}


int random_in_range(int max) {
	assert(max > 0);
	std::uniform_int_distribution<int> distribution(0, max - 1);
	return distribution(*gen);
}


int random_in_range(int min, int max) {
	assert(min < max);
	return min + random_in_range(max - min);
}


double random_real() {
	std::uniform_real_distribution<float> floatDistribution(0.0f, 1.0f);
	return floatDistribution(*gen);
}


bool is_movable_rank(uint8_t rank) {
	assert(rank <= 6);
	return !(rank == RANK_FLAG || rank == RANK_BOMB);
}


// shuffle the first `shuffle_size` elements of an array of size `array_size`
void shuffle(uint8_t* arr, int arr_size, int shuffle_size) {
	for (int i = 0; i < shuffle_size; i++) {
		int idx = random_in_range(i, arr_size);
		std::swap(arr[i], arr[idx]);
	}
}


// completely shuffle an array of size `size`
void shuffle(uint8_t* arr, int size) {
	shuffle(arr, size, size);
}

void pick_random_positions(State* s, int owner) {
	uint8_t unknown_ranks_left = s->ranks_left[owner];
	owner *= 8;

	uint8_t moved_pieces[6], unmoved_pieces[8];
	int nr_moved_pieces = 0, nr_unmoved_pieces = 0;
	for (uint8_t position = 0; position < 100; position++) {
		if (s->board[position] < 0xfe) {
			assert(s->board[position] < 16);
			uint8_t piece_info = s->pieces[s->board[position]];
			assert(piece_info != DEAD);

			if ((s->board[position] & (1 << 3)) == owner)
			{
				uint8_t rank = piece_info & 7;
				if (rank == RANK_UNKNOWN) {
					bool has_moved = piece_info & 8;
					if (has_moved) {
						moved_pieces[nr_moved_pieces++] = position;
					}
					else {
						unmoved_pieces[nr_unmoved_pieces++] = position;
					}
				}
				else {
					uint8_t rank_idx = rank + (rank >= 3 ? 1 : 0);

					if (rank_idx == 2 && !(unknown_ranks_left & 4))
						rank_idx++;

					assert(unknown_ranks_left & (1 << rank_idx));
						
					unknown_ranks_left &= ~(1 << rank_idx);
				}
			}
		}
	}
	assert(nr_moved_pieces <= 6 && nr_unmoved_pieces <= 8);

	uint8_t movable_ranks_left[6], stationary_ranks_left[2];
	int nr_movable_ranks_left = 0, nr_stationary_ranks_left = 0;
	for (int i = 0; i < 8; i++) {
		int rank = i - (i > 2);
		if (unknown_ranks_left & (1 << i)) {
			bool movable = rank != RANK_FLAG && rank != RANK_BOMB;
			if (movable) {
				movable_ranks_left[nr_movable_ranks_left++] = rank;
			}
			else {
				stationary_ranks_left[nr_stationary_ranks_left++] = rank;
			}
		}
	}
	assert(nr_movable_ranks_left <= 6 && nr_stationary_ranks_left <= 2);
	assert(nr_movable_ranks_left + nr_stationary_ranks_left == nr_moved_pieces + nr_unmoved_pieces);
	assert(nr_movable_ranks_left >= nr_moved_pieces && nr_stationary_ranks_left <= nr_unmoved_pieces);


	// assign random ranks to moved pieces
	if (nr_moved_pieces) {
		shuffle(movable_ranks_left, nr_movable_ranks_left, nr_moved_pieces);

		for (int i = 0; i < nr_moved_pieces; i++) {
			s->reveal(s->board[moved_pieces[i]], movable_ranks_left[i]);
		}
	}

	// assign random ranks to rest of the pieces
	if (nr_unmoved_pieces) {
		shuffle(unmoved_pieces, nr_unmoved_pieces);
		for (int i = 0; i < nr_unmoved_pieces; i++) {
			if (i < nr_stationary_ranks_left) {
				s->reveal(s->board[unmoved_pieces[i]], stationary_ranks_left[i]);
			}
			else {
				int idx = i - nr_stationary_ranks_left + nr_moved_pieces;
				assert(0 <= idx && idx < nr_movable_ranks_left);
				s->reveal(s->board[unmoved_pieces[i]], movable_ranks_left[idx]);
			}
		}
	}

	for (int y = 0; y < 10; y++) {
		for (int x = 0; x < 10; x++) {
			int idx = s->board[y * 10 + x];
			if (idx < 16) {
				assert((s->pieces[idx] & 7) != RANK_UNKNOWN);
			}
		}
	}
}


void send_move(uint16_t move) {
	uint8_t from_x = ((move >> 8) & 255) % 10;
	uint8_t from_y = ((move >> 8) & 255) / 10;
	uint8_t to_x = (move & 255) % 10;
	uint8_t to_y = (move & 255) / 10;
	std::cout << "{ \"From\": { \"X\": " << int(from_x) << ", \"Y\": " << int(from_y) << " }, \"To\": { \"X\": " << int(to_x) << ", \"Y\": " << int(to_y) << " } }" << '\n';
}


State init(Bot *bot0, Bot *bot1) {
	StartPosition start_position[2] = { bot0->prepare(0), bot1->prepare(1) };

	State state0;
	for (int i = 0; i < 8; i++) {
		assert(state0.board[start_position[0].position[i]] == CELL_EMPTY);
		state0.board[start_position[0].position[i]] = i;
		state0.pieces[i] = i - (i > 2);

		assert(state0.board[start_position[1].position[i]] == CELL_EMPTY);
		state0.board[start_position[1].position[i]] = i + 8;
		state0.pieces[i + 8] = RANK_UNKNOWN;
	}
	state0.board[42] = state0.board[43] = state0.board[52] = state0.board[53] = CELL_WATER;
	state0.board[46] = state0.board[47] = state0.board[56] = state0.board[57] = CELL_WATER;
	bot0->set_board(state0);

	State state1;
	for (int i = 0; i < 8; i++) {
		assert(state1.board[start_position[0].position[i]] == CELL_EMPTY);
		state1.board[start_position[0].position[i]] = i;
		state1.pieces[i] = RANK_UNKNOWN;

		assert(state1.board[start_position[1].position[i]] == CELL_EMPTY);
		state1.board[start_position[1].position[i]] = i + 8;
		state1.pieces[i + 8] = i - (i > 2);
	}
	state1.board[42] = state1.board[43] = state1.board[52] = state1.board[53] = CELL_WATER;
	state1.board[46] = state1.board[47] = state1.board[56] = state1.board[57] = CELL_WATER;
	assert(state1.board[97] / 8 == 1);
	bot1->set_board(state1);

	State state;
	for (int i = 0; i < 8; i++) {
		assert(state.board[start_position[0].position[i]] == CELL_EMPTY);
		state.board[start_position[0].position[i]] = i;
		state.pieces[i] = i - (i > 2);

		assert(state.board[start_position[1].position[i]] == CELL_EMPTY);
		state.board[start_position[1].position[i]] = i + 8;
		state.pieces[i + 8] = i - (i > 2);
	}
	state.board[42] = state.board[43] = state.board[52] = state.board[53] = CELL_WATER;
	state.board[46] = state.board[47] = state.board[56] = state.board[57] = CELL_WATER;
	return state;
}


uint8_t play_game(Bot *bot0, Bot *bot1, int max_turns, int *num_turns = nullptr) {
	Bot *bot[2] = { bot0, bot1 };
	State state = init(bot0, bot1);

	int turn = 0;
	for (; turn < max_turns; turn++) {
		assert(bot[0]->state.active_player == state.active_player);
		assert(bot[0]->state.ranks_left[0] == state.ranks_left[0]);
		assert(bot[0]->state.ranks_left[1] == state.ranks_left[1]);

		assert(bot[1]->state.active_player == state.active_player);
		assert(bot[1]->state.ranks_left[0] == state.ranks_left[0]);
		assert(bot[1]->state.ranks_left[1] == state.ranks_left[1]);

		uint16_t move = bot[state.active_player]->play_move();
		uint8_t from = move >> 8, to = move & 255;
		uint8_t from_x = from % 10, from_y = from / 10, to_x = to % 10, to_y = to / 10;

		// reveal scout when the piece jumps
		if (abs(from_x - to_x) + abs(from_y - to_y) > 1) {
			assert((state.pieces[state.board[from]] & 7) == RANK_SCOUT);
			assert((bot[state.active_player]->state.pieces[bot[state.active_player]->state.board[from]] & 7) == RANK_SCOUT);
			bot[1 - state.active_player]->reveal(state.board[from], RANK_SCOUT);
		}

		// reveal pieces before battle
		if (state.board[to] != CELL_EMPTY) {
			bot[state.active_player]->reveal(state.board[to], state.pieces[state.board[to]] & 7);
			bot[1 - state.active_player]->reveal(state.board[from], state.pieces[state.board[from]] & 7);
		}

		bot[state.active_player]->do_move(move);
		bot[1 - state.active_player]->do_move(move);

		uint8_t result = state.do_move(move);
		if (result != MOVE_RESULT_NORMAL) {
			if (num_turns) {
				*num_turns = turn;
			}
			return result;  // a move_result can be returned as a game_result
		}
	}

	if (num_turns) {
		*num_turns = turn;
	}

	return GAME_RESULT_DRAW;
}
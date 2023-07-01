#include "evaluation.hpp"

#include "bots/custom_bot.hpp"
#include "bots/random_bot.hpp"
#include "bots/mcts_bot.hpp"

#include <iostream>


int main() {
	init_rand();

	uint8_t start_position[8] = { 0, 2, 31, 35, 11, 39, 10, 1 };
	Bot* bot = new MctsBot(0.35, 30000, [](State *s, int me, int turn) { return simulate(s, me, 8.0, 0.25f); }, start_position);
	bot->prepare(1);

	State state;
	state.board[42] = state.board[43] = state.board[52] = state.board[53] = CELL_WATER;
	state.board[46] = state.board[47] = state.board[56] = state.board[57] = CELL_WATER;

	state.active_player = 1;
	for (int i = 0; i < 8; i++) {
		state.pieces[i] = RANK_UNKNOWN;
		state.pieces[i + 8] = i - (i > 2);
	}

	state.board[62] = 13;
	state.board[63] = 10;
	state.board[78] = 12;
	state.board[89] = 14;
	state.board[96] = 9;
	state.board[98] = 15;
	state.board[99] = 8;
	state.ranks_left[1] = 0xff;

	state.board[4] = 0;
	state.board[5] = 7;
	state.board[27] = 1;
	state.board[30] = 2;
	state.board[31] = 4;
	state.board[36] = 5;
	state.board[39] = 3;
	state.board[64] = 6;

	state.pieces[1] |= MOVE_BIT;
	state.pieces[5] = RANK_GENERAL;
	state.pieces[6] |= MOVE_BIT;

	bot->set_board(state);

	bot->play_move();

	return 0;
}

#include "utils.hpp"
#include "evaluation.hpp"

#include "bots/mcts_bot.hpp"
#include "bots/rejection_bot.hpp"

#include <thread>
#include <iostream>

#define BOTS 2
#define GAMES 300
#define MAX_TURNS 250

int draws[BOTS * BOTS] = {0};
int wins[BOTS * BOTS] = {0};
int losses[BOTS * BOTS] = {0};


void play_games(int lo, int hi) {
	init_rand();

	uint8_t start_position[8] = { 0, 2, 31, 35, 11, 39, 10, 1 };
	Bot *bots[BOTS] = {
		new RejectionBot(0.35, 30000, [](State *s, int me, int turn) { return simulate(s, me, 8.0, 0.35f); }, start_position),
		new MctsBot(0.35, 30000, [](State *s, int me, int turn) { return simulate(s, me, 8.0, 0.25f); }, start_position)
	};

	for (int g = lo, count = 0; g < hi; g++, count++) {
		int gg = g % (BOTS * BOTS);
		int b0 = gg / BOTS;
		int b1 = gg % BOTS;
		assert(b0 < BOTS);
		assert(b1 < BOTS);
		if (b0 != b1) {
			uint8_t result = play_game(bots[b0], bots[b1], MAX_TURNS, 0);
			if (result == GAME_RESULT_DRAW) {
				draws[b0 * BOTS + b1]++;
			}
			if (result == GAME_RESULT_BOT0) {
				wins[b0 * BOTS + b1]++;
			}
			if (result == GAME_RESULT_BOT1) {
				losses[b0 * BOTS + b1]++;
			}
		}
		std::cout << (count + 1) << " / " << (hi - lo + 1) << '\n';
	}
}

int main() {
    std::vector<std::thread> threads;
	const int NUM_THREADS = 16; // std::thread::hardware_concurrency();

	int total_games = GAMES * BOTS * BOTS;
    for (int i = 0; i < NUM_THREADS; ++i) {
		int lo = (total_games * i) / NUM_THREADS;
		int hi = (total_games * (i + 1))/ NUM_THREADS;
        threads.emplace_back(play_games, lo, hi);
    }

    for (auto& thread : threads) {
        thread.join();
    }

	std::cout << "draws\n";
	for (int b0 = 0; b0 < BOTS; b0++) {
		for (int b1 = 0; b1 < BOTS; b1++) {
			std::cout << draws[b0 * BOTS + b1] << '\t';
		}
		std::cout << '\n';
	}
	std::cout << '\n';

	std::cout << "wins\n";
	for (int b0 = 0; b0 < BOTS; b0++) {
		for (int b1 = 0; b1 < BOTS; b1++) {
			std::cout << wins[b0 * BOTS + b1] << '\t';
		}
		std::cout << '\n';
	}
	std::cout << '\n';

	std::cout << "losses\n";
	for (int b0 = 0; b0 < BOTS; b0++) {
		for (int b1 = 0; b1 < BOTS; b1++) {
			std::cout << losses[b0 * BOTS + b1] << '\t';
		}
		std::cout << '\n';
	}
	std::cout << '\n';

	return 0;
}

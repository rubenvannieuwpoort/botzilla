#include "../ibot.hpp"
#include "../utils.hpp"


// simple bot which plays a random move
class RandomBot : public Bot {
public:
	virtual std::string identifier() override {
		return "Random bot";
	}

	RandomBot() : Bot() {
		start_position[0] = 0;  // flag
		start_position[1] = 2;  // spy
		start_position[2] = 31; // scout
		start_position[3] = 35; // scout
		start_position[4] = 11; // miner
		start_position[5] = 39; // general
		start_position[6] = 10; // marshal
		start_position[7] = 1;  // bomb
	};

	virtual uint16_t play_move() override {
		int number_of_moves = enumerate_moves(&state);
		return moves[random_in_range(number_of_moves)];
	}
};

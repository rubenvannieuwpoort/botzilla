#include "bots/rejection_bot.hpp"
#include "bots/mcts_bot.hpp"

#include "evaluation.hpp"

#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <string>
#include <chrono>


json get_msg() {
	std::string line;
	std::getline(std::cin, line);
	return json::parse(line);
}


int parse_gameinit(json msg) {
	return msg["You"];
}


std::string rank_from_num(int n) {
	assert(n <= 7);
	if (n == 0) {
		return "Flag";
	}
	if (n == 1) {
		return "Spy";
	}
	if (n == 2) {
		return "Scout";
	}
	if (n == 3) {
		return "Miner";
	}
	if (n == 4) {
		return "General";
	}
	if (n == 5) {
		return "Marshal";
	}
	if (n == 6) {
		return "Bomb";
	}
	return "???";
}


void send_start_position(StartPosition start_position) {
	std::string result = R"({ "Pieces": [ )";

	for (int i = 0; i < 8; i++) {
		int x = (int)(start_position.position[i] % 10), y = start_position.position[i] / 10;

		if (i > 0) {
			result += ", ";
		}

		result += R"({ "Rank": ")";
		int rank_num = i - (i > 2);
		assert(rank_num < RANK_UNKNOWN);
		result += rank_from_num(rank_num);
		result += R"(", "Position": { "X": )";
		result += std::to_string(x);
		result += R"(, "Y": )";
		result += std::to_string(y);
		result += R"( } })";
	}
	result += R"( ] })";
	
	std::cout << result << '\n';
}


int parse_rank(std::string rank_name) {
	if (rank_name == "Flag") return RANK_FLAG;
	if (rank_name == "Spy") return RANK_SPY;
	if (rank_name == "Scout") return RANK_SCOUT;
	if (rank_name == "Miner") return RANK_MINER;
	if (rank_name == "General") return RANK_GENERAL;
	if (rank_name == "Marshal") return RANK_MARSHAL;
	if (rank_name == "Bomb") return RANK_BOMB;
	assert(rank_name == "?");
	return RANK_UNKNOWN;
}


void update(Bot *bot, json msg) {
	uint8_t from_x = msg["LastMove"]["From"]["X"];
	uint8_t from_y = msg["LastMove"]["From"]["Y"];
	uint8_t to_x = msg["LastMove"]["To"]["X"];
	uint8_t to_y = msg["LastMove"]["To"]["Y"];

	uint8_t from = from_y * 10 + from_x;
	uint8_t to = to_y * 10 + to_x;

	if (msg.contains("BattleResult") && !msg["BattleResult"].is_null()) {
		assert(msg["BattleResult"].contains("Attacker"));
		assert(msg["BattleResult"]["Attacker"].contains("Rank"));
		bot->reveal_at(from, parse_rank(msg["BattleResult"]["Attacker"]["Rank"]));

		assert(msg["BattleResult"].contains("Defender"));
		assert(msg["BattleResult"]["Defender"].contains("Rank"));
		bot->reveal_at(to, parse_rank(msg["BattleResult"]["Defender"]["Rank"]));
	}

	bot->do_move(from * 256 + to);
}


State parse_initial_gamestate(json msg) {
	State state = State{};

	assert(msg["ActivePlayer"] == 0);
	assert(msg["TurnNumber"] == 0);

	int counter[2] = { 0, 8 };

	for (auto cell : msg["Board"]) {
		int x = cell["Coordinate"]["X"], y = cell["Coordinate"]["Y"];

		if (cell["IsWater"]) {
			state.board[y * 10 + x] = CELL_WATER;
			continue;
		}

		if (cell["Rank"].is_null()) {
			continue;
		}

		int owner = cell["Owner"];
		state.board[y * 10 + x] = counter[owner];

		state.pieces[counter[owner]] = parse_rank(cell["Rank"]);
		counter[owner]++;
	}

	state.ranks_left[0] = 0xff;
	state.ranks_left[1] = 0xff;

	return state;
}


int main() {
	init_rand();

	uint8_t start_position[8] = { 0, 2, 31, 35, 11, 39, 10, 1 };
	Bot *bot = new RejectionBot(0.35, 50000, [](State *s, int me, int turn) { return simulate(s, me, 8.0, 0.35f); }, start_position);

	std::cout << "bot-start\n";

	int me = parse_gameinit(get_msg());
	StartPosition sp = bot->prepare(me);
	
	send_start_position(sp);

	State state = parse_initial_gamestate(get_msg());
	bot->set_board(state);


	if (me != STARTING_PLAYER) {
		update(bot, get_msg());
	}

	while (true) {
		assert(bot->state.active_player == me);
		send_move(bot->play_move());
		update(bot, get_msg());  // obtain state after our move
		update(bot, get_msg());  // obtain state after enemy move
	}
}

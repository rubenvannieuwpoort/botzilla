#include "../ibot.hpp"
#include "../node.hpp"
#include "../utils.hpp"

#include <functional>


class RejectionBot : public Bot {
public:
	int iterations;
	double epsilon;
	std::function<float(State *, int me, int turn)> evaluate;

	virtual std::string identifier() override {
		return "Rejection bot";
	}

	RejectionBot(
		double epsilon,
		int iterations,
		std::function<float(State *, int me, int turn)> evaluat,
		uint8_t sp[8]
	) : Bot() {
		this->epsilon = epsilon;
		this->iterations = iterations;
		this->evaluate = evaluat;

		for (int i = 0; i < 8; i++) {
			this->start_position[i] = sp[i];
		}
	};

	virtual uint16_t play_move() override {
		assert(state.active_player == me);

		Node root;

		int count = 0;
		for (int i = 0; i < iterations; i++) {
			count++;
			Node*current = &root;
			State s;

			// rejection sampling based on flag location
			while (true) {
				s = state;
				pick_random_positions(&s, 1 - me);

				int flag_y = -100;
				for (int pos = 0; pos < 100; pos++) {
					if (s.board[pos] < 16 && s.board[pos] / 8 == 1 - me && (s.pieces[s.board[pos]] & 7) == RANK_FLAG) {
						flag_y = pos / 10;
						break;
					}
				}

				if (me == 0) {
					flag_y = 9 - flag_y;
				}
				assert(0 <= flag_y && flag_y <= 3);

				double rejection_probability = flag_y / 6.0;
				if (random_real() >= rejection_probability) {
					break;
				}
			}
			
			// selection
			uint8_t result = MOVE_RESULT_NORMAL;
			while (true) {
				int num_possible_moves = enumerate_moves(&s);

				if (num_possible_moves == 0) {
					if (s.active_player == 0) {
						result = MOVE_RESULT_P1_WIN;
					}
					else {
						result = MOVE_RESULT_P0_WIN;
					}
					break;
				}

				bool there_was_valid_move = false;
				uint16_t mov;

				if (random_real() > epsilon) {
					// pick best node
					float lo_val = 2, hi_val = -2;
					int lo_idx = 0, hi_idx = 0;
					for (int i = 0; i < num_possible_moves; i++) {
						auto result = current->children.find(moves[i]);
						if (result == current->children.end()) {
							continue;
						}

						there_was_valid_move = true;
						float value = result->second.nom / result->second.denom;
						result->second.value = value;
						if (value > hi_val) {
							hi_val = value;
							hi_idx = i;
						}
						if (value < lo_val) {
							lo_val = value;
							lo_idx = i;
						}
					}

					if (s.active_player == me) {
						mov = moves[hi_idx];
					}
					else {
						mov = moves[lo_idx];
					}

				}

				if (!there_was_valid_move) {
					mov = moves[random_in_range(num_possible_moves)];
				}

				uint8_t from = mov >> 8;
				uint8_t to = mov & 255;

				if (current->children.find(mov) == current->children.end()) {
					// expansion: child for this move is missing, add it and exit loop
					Node*new_child = &current->children[mov];
					new_child->parent = current;
					current = new_child;
					result = s.do_move(mov);
					break;
				}

				current = &current->children[mov];
				result = s.do_move(mov);
				if (result != MOVE_RESULT_NORMAL) {
					break;  // we have reached a terminal node
				}
			}

			// simulation
			float nom, denom;
			if (result) {
				// for terminal node
				if (result == MOVE_RESULT_DRAW) {
					nom = .5;
					denom = 1;
				}
				else if ((result == MOVE_RESULT_P0_WIN && me == 0) || (result == MOVE_RESULT_P1_WIN && me == 1)) {
					nom = 1;
					denom = 1;
				}
				else {
					assert((result == MOVE_RESULT_P0_WIN && me == 1) || (result == MOVE_RESULT_P1_WIN && me == 0));
					nom = 0;
					denom = 1;
				}
			}
			else {
				// for non-terminal node
				nom = evaluate(&s, me, state.turn);
				denom = 1;
			}

			// backpropagation
			while (current) {
				current->nom += nom;
				current->denom += denom;
				current->value = current->nom / current->denom;
				current = current->parent;
			}
		}

		int lo_index = 0, hi_index = 0;
		float lo = 2, hi = -2;
		for (auto kv : root.children) {
			float value = kv.second.nom / kv.second.denom;
			if (value > hi) {
				hi_index = kv.first;
				hi = value;
			}
			if (value < lo) {
				lo_index = kv.first;
				lo = value;
			}
		}

		return hi_index;
	}
};

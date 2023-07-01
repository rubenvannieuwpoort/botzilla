#pragma once

#include <cstdint>
#include <unordered_map>


struct Node {
	float value;
	float nom = 0, denom = 0;
	std::unordered_map<uint16_t, Node> children;
	Node *parent = nullptr;
};

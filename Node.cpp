#include "Node.h"
#include "Graph.h"

Node::Node()
{
	id = new int(-1);
	visited = new bool(false);
	enabled = new bool(true); 
	occupied = new bool(false);
	up = nullptr;
	down = nullptr;
	right = nullptr;
	left = nullptr;
	isShipment = new bool(false);
}

Node::~Node()
{
// TODO: Throws access violation exception related to building tile nodes. 
}

void Node::addEdge(Node* node, Direction direction)
{
	switch (direction) {
	case Direction::UP:
		up = node;
		break;
	case Direction::DOWN:
		down = node;
		break;
	case Direction::RIGHT:
		right = node;
		break;
	case Direction::LEFT:
		left = node;
		break;
	}
}
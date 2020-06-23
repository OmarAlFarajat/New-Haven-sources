#include "TileNode.h"

// TODO: Consider making all of this header-only? 

TileNode::TileNode()
{
	resources[0] = nullptr;
	resources[1] = nullptr;
	resources[2] = nullptr;
	resources[3] = nullptr;

	isShipment = nullptr;
}

TileNode::~TileNode()
{
	for (auto each : resources) {
		if (each) {
			delete each;
			each = nullptr;
		}
	}
	if (resources) {
		delete[] resources;
	}
}

void TileNode::linkResourceNode(Resource* resource, int index)
{
	resources[index] = resource; 
}
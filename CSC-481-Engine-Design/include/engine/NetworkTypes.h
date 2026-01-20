#pragma once
#include "Types.h"  // for OrderedPair
#include <vector>

// Client info sent to server
struct ClientCommand {
	int clientId;
	uint32_t actions; // 32 bit action mask, lets each game set an action to a bit
	int tick;
	float x;
	float y;
};

// Stored on the server
struct PlayerState {
	int tick = 0;
	uint32_t actions = 0;
	float x = 0.0f;
	float y = 0.0f;
};

// Synced object data
struct SyncedObjectData {
    int id;
    int type;
    OrderedPair position;
};

// World snapshot sent from server to client
struct WorldSnapshot {
	int tick;
	std::vector<int> playerIds;
	std::vector<OrderedPair> playerPositions;
	std::vector<SyncedObjectData> syncedObjects; // Changed from autoPositions
};

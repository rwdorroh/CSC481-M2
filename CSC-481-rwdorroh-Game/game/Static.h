#pragma once
#include <engine/Entity.h>

// The Static class represents non-moving, collidable objects in the game world
class Static : public Entity {
public:

	// Constructor for Static objects
	Static(float x, float y, float w, float h, const char* texturePath);

};
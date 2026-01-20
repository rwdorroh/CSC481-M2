#include "Auto.h"
#include <engine/Engine.h>
#include <cmath>

// Auto constructor sets the direction toward the Player with no gravoty and collidable
Auto::Auto(float x, float y, float w, float h, const char* texturePath)
    : Entity(x, y, w, h, texturePath, false, true) {

	// Default local direction (only used if !serverControlled)
	direction = { -1.0f, 0.45f };
	float norm = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (norm > 0.0001f) { direction.x /= norm; direction.y /= norm; }
}

void Auto::setServerControlled(bool enabled) {
	serverControlled = enabled;
	if (serverControlled) {
		// Clear any local velocity so we never "double move" on client
		setVelocity({ {0,0}, 0.0f });
	}
	else {
		// Reapply local velocity if we ever toggle back
		setVelocity({ direction, speed });
	}
}

// Update function for auto moving object, respawns once off screen
void Auto::update(float deltaTime) {

	if (serverControlled) {
		// Do NOTHING locally. Server snapshots set the position each frame.
		return;
	}

	// (Optional offline mode) local behavior:
	Entity::update(deltaTime);

	// local-only respawn
	const float w = getRect().w;
	const OrderedPair pos = getPosition();
	if (pos.x + w < 0 || pos.y > 1080) {
		setPosition({ 1920.0f - w, 0.0f });
		setVelocity({ direction, speed });
	}
}

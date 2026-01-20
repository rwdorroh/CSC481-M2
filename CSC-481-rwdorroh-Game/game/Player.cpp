#include "Player.h"
#include "Actions.h"
#include "Static.h"  
#include "Auto.h"
#include <engine/Collision.h>
#include <engine/Physics.h>
#include <engine/Input.h>
#include <engine/Engine.h>
#include <vector>


// Player entity with gravity and collision on by default
Player::Player(float x, float y, float w, float h, const char* texturePath)
    : Entity(x, y, w, h, texturePath, true, true) {
}

void Player::update(float deltaTime) {

	if (paused) return;
	
	// Dodge timer
	if (dodgeActive) {
		dodgeTimer -= deltaTime;
		if (dodgeTimer <= 0.0f) {
			dodgeActive = false;
		}
	}

	// Apply physics
	Entity::update(deltaTime);

	
	isOnGround = false;

	// Platform collisions first to compute isOnGround
	{
		const std::vector<Entity*> ents = Engine::getEntitiesSnapshot();
		for (Entity* e : ents) {
			if (e == this || !e->isCollidable()) continue;

			// Only treat Static as ground/platform
			if (auto* platform = dynamic_cast<Static*>(e)) {
				const float platformTop = platform->getPosition().y;
				const float playerBottom = getPosition().y + getRect().h;

				if (playerBottom >= platformTop && Collision::checkCollision(*this, *platform)) {
					// Snap to top and zero vertical velocity
					OrderedPair pos = getPosition();
					pos.y = platformTop - getRect().h;
					setPosition(pos);

					Velocity v = getVelocity();
					v.direction = { 0, 0 };
					v.magnitude = 0.0f;
					setVelocity(v);

					isOnGround = true;
				}
			}
		}
	}

	// Handle actions now 
	const uint32_t actions = getPendingActions();
	if ((actions & ACTION_DODGE) && !dodgeActive) {
		startDodge();
	}
	if (isOnGround && (actions & ACTION_JUMP)) {
		jump();
	}
	setPendingActions(0); // clear for next frame

	// Orb collisions after dodge is possibly active
	{
		const std::vector<Entity*> ents = Engine::getEntitiesSnapshot();
		for (Entity* e : ents) {
			if (e == this || !e->isCollidable()) continue;

			// Only treat Auto as the hazard
			if (auto* orb = dynamic_cast<Auto*>(e)) {
				if (Collision::checkCollision(*this, *orb) && !dodgeActive) {
					handleCollision(*orb); // respawn only if not dodging
				}
			}
		}
	}
}


// Handles collision with the orb by resetting player position and velocity
void Player::handleCollision(const Entity& other) {
	// Reset position/velocity on orb hit
	setPosition({ 300.0f, 500.0f });
	setVelocity({ {0, 0}, 0 });
}

// Initiates the dodge state
void Player::startDodge() {
    dodgeActive = true;
    dodgeTimer = dodgeDuration;
}

// Makes the player jump if on the ground
void Player::jump() {
    Velocity v = getVelocity();
    v.direction.x = 0;
    v.direction.y = -1;  // Upward direction
    v.magnitude = 500.0f;  // Tune this value to jump height
    setVelocity(v);
}

void Player::setPaused(bool p) {
	paused = p;
	if (paused) {
		// Zero out velocity so you truly "hang" midair.
		Velocity v = getVelocity();
		v.direction = { 0, 0 };
		v.magnitude = 0.0f;
		setVelocity(v);
	}
}

#pragma once
#include <engine/Entity.h>

class Player : public Entity {
public:
    Player(float x, float y, float w, float h, const char* texturePath);

    void update(float deltaTime) override;

	void setPaused(bool p);

private:
    bool isOnGround = false;

    // Dodge state
    bool dodgeActive = false;
    float dodgeTimer = 0.0f;
    const float dodgeDuration = 3.0f; // seconds

    // Inputs
    void jump();
    void startDodge();

    void handleCollision(const Entity& other);

	bool paused = false;

};
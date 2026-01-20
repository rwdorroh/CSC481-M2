#pragma once

#include "Entity.h"

// The Physics class manages physics-related operations like gravity.
// It is a static class because it doesn't need to store per-object state.
class Physics {
public:
    // Sets the global gravity strength.
    static void setGravity(float gravity);
    // Gets the current gravity strength.
    static float getGravity();
    // Applies gravity and velocity to an entity.
    static void apply(Entity* entity, float deltaTime);
private:
    // The global gravity value.
    static float gravityWeight;
};

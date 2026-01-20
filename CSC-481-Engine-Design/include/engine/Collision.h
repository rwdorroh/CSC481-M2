#pragma once

#include <engine/Entity.h>

// The Collision class provides a static function for collision detection.
// It is a static class as it does not need to store any state.
class Collision {
public:
    // A simple collision check.
    // This function uses the `getRect()` method from the Entity class
    // for a clean and efficient collision check.
    static bool checkCollision(const Entity& a, const Entity& b);
};

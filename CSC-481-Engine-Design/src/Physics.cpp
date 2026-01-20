#include <engine/Physics.h>
#include <engine/Types.h>
#include <engine/Entity.h>
#include <cmath>

// Static gravity value
float Physics::gravityWeight = 9.81f;

/**
 * Sets the global gravity weight.
 * @param gravity The new gravity value.
 */
void Physics::setGravity(float gravity) {
    gravityWeight = gravity;
}

/**
 * Gets the current global gravity weight.
 * @return The current gravity value.
 */
float Physics::getGravity() {
    return gravityWeight;
}

/**
 * Applies gravity to an entity by adjusting its vertical velocity.
 * This function is called every frame for each entity that is affected by gravity.
 * @param entity A pointer to the entity to apply gravity to.
 * @param deltaTime The time elapsed since the last frame, used to make the physics
 * simulation independent of the frame rate.
 */
void Physics::apply(Entity* entity, float deltaTime) {
    if (entity && entity->isAffectedByGravity()) {
        Velocity v = entity->getVelocity();
        // Convert velocity vector to components
        float vx = v.direction.x * v.magnitude;
        float vy = v.direction.y * v.magnitude;
        // Add gravity acceleration to vertical component (positive Y is down)
        vy += gravityWeight * deltaTime;
        // Recalculate magnitude and normalize direction
        float mag = std::sqrt(vx * vx + vy * vy);
        if (mag > 0.0001f) {
            v.direction.x = vx / mag;
            v.direction.y = vy / mag;
            v.magnitude = mag;
        } else {
            v.direction.x = 0.0f;
            v.direction.y = 0.0f;
            v.magnitude = 0.0f;
        }
        entity->setVelocity(v);
    }
}

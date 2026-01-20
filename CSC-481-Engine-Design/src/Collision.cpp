#include <engine/Collision.h>

/**
 * Checks for a collision between two entities using detection.
 * This is a simple and fast method that checks for an overlap between the two entities' rectangular
 * bounding boxes on both the X and Y axes.
 * @param a The first entity.
 * @param b The second entity.
 * @return true if the two entities are overlapping, false otherwise.
 */
bool Collision::checkCollision(const Entity& a, const Entity& b) {
    // Get the bounding rectangles for both entities.
    SDL_FRect rectA = a.getRect();
    SDL_FRect rectB = b.getRect();
    // Check if there is an overlap on the X axis.
    bool xOverlap = (rectA.x < rectB.x + rectB.w) && (rectA.x + rectA.w > rectB.x);
    // Check if there is an overlap on the Y axis.
    bool yOverlap = (rectA.y < rectB.y + rectB.h) && (rectA.y + rectA.h > rectB.y);
    // Return true only if there is an overlap on both axes.
    return xOverlap && yOverlap;
}

#include <engine/Entity.h>
#include <engine/Engine.h>
#include <engine/Physics.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

/**
 * Constructs an Entity.
 * @param x Initial X position.
 * @param y Initial Y position.
 * @param w Width of the entity.
 * @param h Height of the entity.
 * @param texturePath Path to the texture image file.
 * @param affectedByGravity Determines if the entity is subject to the physics system.
 * @param collidable Determines if the entity can be checked for collisions.
 * @param pending actions of a client entity
 * @param pending tick of a client entity
 */
Entity::Entity(float x, float y, float w, float h, const char* texturePath, bool affectedByGravity, bool collidable)
    : position{x, y}, dimensions{w, h}, velocity{{0.0f, 0.0f}, 0.0f},
      applyGravity(affectedByGravity), collidable(collidable), 
	pendingActions(0), pendingTick(0) 
{

    SDL_Renderer* renderer = Engine::getRenderer();
	
    if (renderer) {
        // Load the image file into an SDL surface
        SDL_Surface* surface = IMG_Load(texturePath);

        if (surface) {

            // Create a texture from the surface for efficient rendering.
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_DestroySurface(surface);

        } else {
            texture = nullptr;
            std::cerr << "Failed to load surface from " << texturePath << ": " << SDL_GetError() << std::endl;
        }
    } else {
        texture = nullptr;
        std::cerr << "Renderer is null, cannot load texture." << std::endl;
    }
}

/**
 * Destroys the Entity and cleans up its texture.
 */
Entity::~Entity() {
    if (texture) {
        SDL_DestroyTexture(texture);
    }
}

/**
 * Updates the entity's position based on its velocity and gravity.
 * @param deltaTime The time elapsed since the last frame.
 */
void Entity::update(float deltaTime) {
    if (applyGravity) {
        Physics::apply(this, deltaTime);
    }
    // Apply velocity to the position.
    position.x += velocity.direction.x * velocity.magnitude * deltaTime;
    position.y += velocity.direction.y * velocity.magnitude * deltaTime;
}

/**
 * Draws the entity's texture on the screen.
 */
void Entity::draw() {
    SDL_Renderer* renderer = Engine::getRenderer();
    if (renderer && texture) {
        // Get the bounding box rectangle for the entity.
        SDL_FRect rect = getRect();
        // Render the texture to the screen at the entity's position.
        SDL_RenderTexture(renderer, texture, nullptr, &rect);
    }
}

// All getter and setter methods below provide a clean interface
// for the game to interact with the entity's properties.
void Entity::setVelocity(const Velocity& v) {
    velocity = v;
}

Velocity Entity::getVelocity() const {
    return velocity;
}

void Entity::setPosition(const OrderedPair& p) {
    position = p;
}

OrderedPair Entity::getPosition() const {
    return position;
}

void Entity::setAffectedByGravity(bool enabled) {
    applyGravity = enabled;
}

bool Entity::isAffectedByGravity() const {
    return applyGravity;
}

void Entity::setCollidable(bool enabled) {
    collidable = enabled;
}

bool Entity::isCollidable() const {
    return collidable;
}

void Entity::setPendingActions(uint32_t mask) {
	pendingActions = mask;
}

uint32_t Entity::getPendingActions() const {
	return pendingActions;
}

void Entity::setPendingTick(int t) {
	pendingTick = t;
}

int Entity::getPendingTick() const {
	return pendingTick;
}

/**
 * Returns the entity's bounding box as an SDL_FRect.
 * @return An SDL_FRect with the entity's position and dimensions.
 */
SDL_FRect Entity::getRect() const {
    return {position.x, position.y, dimensions.x, dimensions.y};
}

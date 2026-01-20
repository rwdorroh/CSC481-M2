#pragma once

#include <SDL3/SDL.h>
#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <engine/Types.h>
// Calls the entity class to make it known that it is using it
class Entity;

// The core engine class. It manages the game loop, window, renderer, and entities.
class Engine {
public:
    // This is the corrected Config struct, nested inside the Engine class.
    struct Config {
        const char* title;
        int width;
        int height;
    };
	// Runs the main game loop.
	static void run(std::function<void(float)> update, std::function<void(void)> render);

	// Add an entity to the engine.
	static void addEntity(Entity* entity);

	// Getter for entities, deadlock causing
	// static const std::vector<Entity*>& getEntities();

	// Safe access: returns a copy/snapshot of the entity list with no lock held by the caller.
	static std::vector<Entity*> getEntitiesSnapshot();

	// Getters for the renderer.
	static SDL_Renderer* getRenderer();

    // Initializes the engine
	static bool init(const Config& cfg);

    // Shuts down the engine and cleans up all resources.
	static void shutdown();

	// Get the mutex, if needed in game
	static std::mutex& getEntitiesMutex();

private:
	// Private members for the engine's core functionality.
	static SDL_Window* s_window;
	static SDL_Renderer* s_renderer;
	static bool s_running;
	static std::vector<Entity*> s_entities;

	// Multithreading private members
	static std::thread s_updateThread;
	static std::mutex  s_entitiesMutex;
	static std::atomic<bool> s_workerRunning;
};

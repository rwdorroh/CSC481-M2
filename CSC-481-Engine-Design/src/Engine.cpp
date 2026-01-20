#include <engine/Engine.h>
#include <engine/Input.h>
#include <engine/Physics.h>
#include <engine/Collision.h>
#include <engine/Entity.h>
#include <SDL3/SDL.h>
#include <iostream>

// Static members initialization, and core components for the engine working
SDL_Window* Engine::s_window = nullptr;
SDL_Renderer* Engine::s_renderer = nullptr;
bool Engine::s_running = false;
std::vector<Entity*> Engine::s_entities;

// Static thread member initialization
std::thread Engine::s_updateThread;
std::mutex  Engine::s_entitiesMutex;
std::atomic<bool> Engine::s_workerRunning = false;

/**
 * Initializes the SDL and events, and creates the game window and renderer.
 * @param cfg The configuration struct containing window title, width, and height.
 * @return true if initialization is successful, false otherwise.
 */
bool Engine::init(const Config& cfg) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return false;
	}
    // Create the game window.
	s_window = SDL_CreateWindow(cfg.title, cfg.width, cfg.height, SDL_WINDOW_RESIZABLE);
	if (!s_window) {
		SDL_Log("Couldn't create window: %s", SDL_GetError());
		return false;
	}
    // Create the renderer for drawing.
	s_renderer = SDL_CreateRenderer(s_window, nullptr);
	if (!s_renderer) {
		SDL_Log("Couldn't create renderer: %s", SDL_GetError());
		return false;
	}
	return true;
}

/**
 * Cleans up all resources used by the engine.
 * This includes deleting all entities, destroying the renderer and the window.
 */
void Engine::shutdown() {

	// stop worker first (if runThreaded was used)
	s_workerRunning = false;
	if (s_updateThread.joinable()) s_updateThread.join();

	{	// Clean up all allocated entity objects
		std::lock_guard<std::mutex> lock(s_entitiesMutex); // lock while destroying entities
		for (Entity* entity : s_entities) {
			delete entity;
		}
		s_entities.clear();
	}
	SDL_DestroyRenderer(s_renderer);
	SDL_DestroyWindow(s_window);
	SDL_Quit();
}

/**
 * Adds a new entity to the engine's list of managed entities.
 * @param entity A pointer to the entity to add.
 */
void Engine::addEntity(Entity* entity) {
	std::lock_guard<std::mutex> lock(s_entitiesMutex); // lock for adding entities
	s_entities.push_back(entity);
}

/**
 * The main game loop. It handles events, updates game state, and renders the scene.
 * @param update The function to call for game state updates.
 * @param render The function to call for custom render logic.
 */
void Engine::run(std::function<void(float)> update, std::function<void(void)> render) {
	
	s_running = true;
	s_workerRunning = true; 

	// Worker thread: updates all entities with its own dt
	s_updateThread = std::thread([&]() {
		Uint64 last = SDL_GetTicks();
		while (s_workerRunning) {
			Uint64 now = SDL_GetTicks();
			float dt = (now - last) / 1000.0f;
			last = now;

			// Take a snapshot under lock, then release the lock before calling update()
			std::vector<Entity*> snapshot;
			{
				std::lock_guard<std::mutex> lock(s_entitiesMutex);
				snapshot = s_entities;
			}

			for (Entity* e : snapshot) {
				if (e) e->update(dt);
			}

			// Sleep for 1 ms
			SDL_Delay(1);
		}
		});

	// Main thread: events, input, game update (network/timeline), render
	SDL_Event e;
	Uint64 lastTime = SDL_GetTicks();// Get initial time for delta time calculation
	while (s_running) {
        // Process all pending SDL events
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_EVENT_QUIT) {
				s_running = false;
			} 
		}
        Input::updateKeyboardState();

        // Calculate delta time
		Uint64 currentTime = SDL_GetTicks();
		float deltaTime = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;

		update(deltaTime);

		SDL_SetRenderDrawColor(s_renderer, 255, 255, 255, 255);  // white background
		SDL_RenderClear(s_renderer);
		
		// Draw from a snapshot (do not hold the lock during draw())
		std::vector<Entity*> drawSnapshot;
		{
			std::lock_guard<std::mutex> lock(s_entitiesMutex); // lock while drawing
			drawSnapshot = s_entities;
		}
		for (Entity* entity : drawSnapshot) {
			if (entity) entity->draw();
		}

		render(); // font does work with it here
		SDL_RenderPresent(s_renderer);
	}

	// Shutdown worker
	s_workerRunning = false;
	if (s_updateThread.joinable()) s_updateThread.join();
}

/**
 * Provides access to the global SDL renderer instance.
 * @return A pointer to the SDL_Renderer.
 */
SDL_Renderer* Engine::getRenderer() {
	return s_renderer;
}

// Gets the entity mutex
std::mutex& Engine::getEntitiesMutex() { 
	return s_entitiesMutex; 
}

// Deadlock safe get entities
std::vector<Entity*> Engine::getEntitiesSnapshot() {
	std::lock_guard<std::mutex> lock(s_entitiesMutex);
	return s_entities; // copy
}

/**
 * Provides access to the list of entities managed by the engine.
 * @return A constant reference to the vector of entity pointers.
 */
 //const std::vector<Entity*>& Engine::getEntities() {
	 //std::lock_guard<std::mutex> lock(s_entitiesMutex); // lock for getting entities
	 //return s_entities;
 //}

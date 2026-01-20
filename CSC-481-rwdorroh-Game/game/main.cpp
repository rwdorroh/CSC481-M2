#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

#include "Static.h"
#include "Player.h"
#include "Auto.h"
#include "Actions.h"

#include <engine/Engine.h>
#include <engine/Input.h>
#include <engine/Physics.h>
#include <engine/Client.h>
#include <engine/Timeline.h>
#include <engine/NetworkTypes.h>


// Global font pointer for HUD
TTF_Font* hudFont = nullptr;

// Timeline speed levels
const std::vector<float> speedLevels = { 0.5f, 1.0f, 2.0f };
size_t currentSpeedIndex = 1;

// Helper function to render text to a texture
SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), text.length(), color);
    if (!surface) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return texture;
}

// Mutex + snapshot storage
std::mutex stateMutex;
struct ServerSnapshot {
	std::unordered_map<int, OrderedPair> otherPlayersPositions;
	std::vector<SyncedObjectData> syncedObjects;
	bool valid = false;
};
ServerSnapshot latestSnapshot;

// Bind actions for my game
void setupInputBindings() {
	Input::clearBindings();

	// Networked actions
	Input::bindAction(SDL_SCANCODE_W, 0); // Jump → bit 0
	Input::bindAction(SDL_SCANCODE_S, 1); // Dodge → bit 1
	Input::bindAction(SDL_SCANCODE_UP, 2); // Scale up
	Input::bindAction(SDL_SCANCODE_DOWN, 3); // Scale down
	Input::bindAction(SDL_SCANCODE_SPACE, 4); // Pause
}

// Network thread
void networkReceiveThread(Client& net, int playerID) {
	while (true) {
		WorldSnapshot snapshot;
		if (!net.pollUpdate(snapshot)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		std::lock_guard<std::mutex> lock(stateMutex);
		latestSnapshot.syncedObjects = snapshot.syncedObjects;
		latestSnapshot.otherPlayersPositions.clear();
		for (size_t i = 0; i < snapshot.playerIds.size(); ++i) {
			if (snapshot.playerIds[i] != playerID) {
				latestSnapshot.otherPlayersPositions[snapshot.playerIds[i]] =
					snapshot.playerPositions[i];
			}
		}
		latestSnapshot.valid = true;
	}
}

int main(int argc, char* argv[]) {

    // Ask for a player ID so each client is unique
    int playerID;
	std::cout << "Enter player ID (integer): ";
	std::cin >> playerID;

    // Set clientID with playerID
	Client::setClientID(playerID);

	// Configure the engine window
    Engine::Config config;
	config.title = "CSC 481 Game";
	config.width = 1900;
	config.height = 1000;

	// TTF initialization
    if (TTF_Init() < 0) {
        SDL_Log("Failed to init TTF: %s", SDL_GetError());
        return 1;
    }

	// Load font for HUD
    hudFont = TTF_OpenFont("assets/DejaVuSans.ttf", 24);
    if (!hudFont) {
        SDL_Log("Failed to load font: %s", SDL_GetError());
        return 1;
    }

    // Initialize the engine
    if (!Engine::init(config)) {
        SDL_Log("Failed to initialize engine: %s", SDL_GetError());
        return 1;  // Failed to init SDL
    }

	// Setup input bindings (jump + dodge)
	setupInputBindings();

	// Increase gravity for this session
	Physics::setGravity(200.0f);

    // Initialize the timeline
    Timeline timeline;
    timeline.init();

	// Set initial time scale
	timeline.setScale(speedLevels[currentSpeedIndex]);
	setupInputBindings();

	// Initialize the client for networking
	Client net;
	bool isConnected = net.connect();

	// Static platform
	Engine::addEntity(new Static(300.0f, 800.0f, 96.0f, 32.0f, "assets/Brick.png"));

	// Local player
	Player* localPlayer = new Player(300.0f, 500.0f, 64.0f, 64.0f, "assets/Morwen.png");
	Engine::addEntity(localPlayer);

	// Map of other players in the game
	std::unordered_map<int, Player*> otherPlayers;

	// Auto-moving orb reference
	Entity* orb = nullptr;

	// Network receive thread
	std::thread netThread;
	if (isConnected) netThread = std::thread(networkReceiveThread, std::ref(net), playerID);

	int currentTick = 0;
	bool wasScaleUp = false, wasScaleDown = false, wasPause = false;

    // Main game loop
    Engine::run(
        [&](float rawDelta) {

			const uint32_t actionMask = Input::getActionMask();
            
            // Speed up with up arrow
			bool scaleUp = (actionMask & ACTION_SCALE_UP);
			if (scaleUp && !wasScaleUp && currentSpeedIndex < speedLevels.size() - 1) {
				timeline.setScale(speedLevels[++currentSpeedIndex]);
			}
			wasScaleUp = scaleUp;

			// Slow down with down arrow
			bool scaleDown = (actionMask & ACTION_SCALE_DOWN);
			if (scaleDown && !wasScaleDown && currentSpeedIndex > 0) {
				timeline.setScale(speedLevels[--currentSpeedIndex]);
			}
			wasScaleDown = scaleDown;

			// Handle pausing with Space
			bool pause = (actionMask & ACTION_PAUSE);
			if (pause && !wasPause) {
				if (timeline.isPaused()) {
					timeline.resume();
					localPlayer->setPaused(false);
				}
				else {
					timeline.pause();
					localPlayer->setPaused(true);
				}
			}
			wasPause = pause;

            // Update the scaled timeline
            float scaledDelta = static_cast<float>(timeline.update());
			currentTick++;

			if (timeline.isPaused()) {
				return; // Skip all updates while paused
			}

			// Local player actions
			localPlayer->setPendingActions(actionMask);
			localPlayer->update(scaledDelta);

			// Apply latest snapshot to orb + other players
			{
				std::lock_guard<std::mutex> lock(stateMutex);
				if (latestSnapshot.valid) {
					for (const auto& obj : latestSnapshot.syncedObjects) {
						if (obj.id == 1 && obj.type == 1) { // Orb
							if (!orb) {
								orb = new Auto(obj.position.x, obj.position.y, 128, 128, "assets/Orb.png");
								static_cast<Auto*>(orb)->setServerControlled(true);
								Engine::addEntity(orb);
							}
							else {
								orb->setPosition(obj.position);
							}
						}
					}
					for (auto& [id, pos] : latestSnapshot.otherPlayersPositions) {
						if (otherPlayers.find(id) == otherPlayers.end()) {
							Player* np = new Player(pos.x, pos.y, 64, 64, "assets/Morwen.png");
							otherPlayers[id] = np;
							Engine::addEntity(np);
						}
						else {
							otherPlayers[id]->setPosition(pos);
						}
					}
				}
			}

			// Send player state to server
			if (isConnected) {
				ClientCommand cmd{ playerID, currentTick, actionMask,
								  localPlayer->getPosition().x, localPlayer->getPosition().y };
				net.sendCommand(cmd);
			}
			localPlayer->setPendingActions(0);
        },
        [&]() {
			SDL_Renderer* renderer = Engine::getRenderer();
			SDL_Color black = { 0,0,0,255 };
			std::stringstream ss;
			ss << "Client ID: " << playerID << " | Speed: x" << timeline.getScale();
			if (timeline.isPaused()) ss << " [PAUSED]";
			SDL_Texture* tex = renderText(renderer, hudFont, ss.str(), black);
			if (tex) {
				float w, h; SDL_GetTextureSize(tex, &w, &h);
				SDL_FRect dst = { 10, 10, w, h };
				SDL_RenderTexture(renderer, tex, NULL, &dst);
				SDL_DestroyTexture(tex);
			}
		}
    );

	// Shutdown the engine and clean up resources
	if (isConnected && netThread.joinable()) netThread.detach();
    if (hudFont) {
        TTF_CloseFont(hudFont);
    }
    TTF_Quit();
    Engine::shutdown();
    return 0;
}

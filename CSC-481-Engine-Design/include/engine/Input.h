#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <unordered_map>

class Input {
public:
    // Asks if a key is currently being pressed.
    static bool isKeyPressed(SDL_Scancode key);

    // Grabs the latest keyboard state. Call this once per frame.
    static void updateKeyboardState();

	// Bind a key to an action bit index (0 to 31)
	static void bindAction(SDL_Scancode key, uint32_t bit);

	// Clear all bindings
	static void clearBindings();

	// Build an action mask for this fram
	static uint32_t getActionMask();

private:
    // A list of all keys on the keyboard and whether each one is pressed or not.
    static const bool* keyboardState;

	// Map from SDL key to which bit it controls
	static std::unordered_map<SDL_Scancode, uint32_t> keyBindings;
};
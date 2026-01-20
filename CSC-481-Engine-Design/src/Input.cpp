#include <engine/Input.h>
#include <SDL3/SDL.h>

// Initializes the static keyboardState pointer to null.
// This pointer will be used by SDL with the current keyboard state.
const bool* Input::keyboardState = nullptr;

// Key map
std::unordered_map<SDL_Scancode, uint32_t> Input::keyBindings;

/**
 * Updates the internal keyboard state by getting the latest state.
 * This function should be called once per frame in the main game loop
 * to ensure that the keyboard state is always up-to-date.
 */
void Input::updateKeyboardState() {
    keyboardState = SDL_GetKeyboardState(nullptr);
}

/**
 * Checks the stored keyboard state to see if a specific key is pressed.
 * @param key The SDL_Scancode of the key to check.
 * @return true if the key is currently pressed, false otherwise.
 */
bool Input::isKeyPressed(SDL_Scancode key) {
    if (keyboardState != nullptr) {
        return keyboardState[key];
    }
    return false;
}

// Bind key to bit
void Input::bindAction(SDL_Scancode key, uint32_t bit) {
	keyBindings[key] = bit;
}

// Clear key to bit bindings
void Input::clearBindings() {
	keyBindings.clear();
}

// Get the action mask for the game
uint32_t Input::getActionMask() {
	uint32_t mask = 0;
	if (keyboardState != nullptr) {
		for (auto& [key, bit] : keyBindings) {
			if (keyboardState[key]) {
				mask |= (1u << bit);
			}
		}
	}
	return mask;
}

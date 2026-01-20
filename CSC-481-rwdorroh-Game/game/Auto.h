#pragma once
#include <engine/Entity.h>

class Auto : public Entity {
public:
    Auto(float x, float y, float w, float h, const char* texturePath);

    void update(float deltaTime) override;

	void setServerControlled(bool enabled);
	bool isServerControlled() const { return serverControlled; }


private:
    OrderedPair direction{};
    float speed = 300.0f; // Adjust as needed

	bool serverControlled = true;

	// record spawn anchor to keep formula stable
	float startX = 1920.0f - 128.0f; // match your spawn
	float startY = 0.0f;
};

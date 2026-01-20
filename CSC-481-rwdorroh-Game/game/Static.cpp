#include "Static.h"

// Static objects are not affected by gravity and are collidable by default
Static::Static(float x, float y, float w, float h, const char* texturePath)
    : Entity(x, y, w, h, texturePath, false, true)
{}
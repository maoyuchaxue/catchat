#include "World.h"
#include "const.h"

int main() {
    World* world = World::getWorld();
    world->init();
}
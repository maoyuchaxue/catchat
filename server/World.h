#ifndef WORLD_H
#define WORLD_H

#include "User.h"
#include <vector>

using namespace std;

class World {
private:
    vector<User*> users;
    static World* instance;
    int main_socketfd;

    World();
public:
    void init();
    static World *getWorld();
};

#endif

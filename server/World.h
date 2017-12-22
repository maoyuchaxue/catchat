#ifndef WORLD_H
#define WORLD_H

class World;

#include "User.h"
#include <vector>
#include <pthread.h>
#include <string>

using namespace std;

class World {
private:
    vector<User*> users;
    static World* instance;
    int main_socketfd;

    pthread_mutex_t users_mutex;
    World();

public:
    void init();
    static World *getWorld();
    void addUser(User *user);
    void removeUser(int socketid);
    string getAllUserString();
    bool handleSendMessage(User *sender, string srecv);
};

#endif

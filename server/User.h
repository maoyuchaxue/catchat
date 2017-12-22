
#ifndef USER_H
#define USER_H

class User;

#include <vector>
#include "World.h"
#include <pthread.h>

class User {
private:
    int socketid;
    pthread_t thread_id;
    World *world;
    static void *run(void *user);

public:
    User(World *_world, int _socketid); 
    void startThread();
};

#endif
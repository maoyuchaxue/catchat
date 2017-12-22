#include "User.h"
#include <pthread.h>

User::User(World *_world, int _socketid) {
    world = _world;
    socketid = _socketid;
}

void User::startThread() {
    pthread_create(&thread_id, NULL, &run, (void *)(this));
}

void* User::run(void *user) {
    User *that = (User *)user;
}
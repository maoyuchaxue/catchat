#ifndef WORLD_H
#define WORLD_H

class World;

#include "User.h"
#include <vector>
#include <pthread.h>
#include <string>
#include <map>

using namespace std;

class World {
private:
    vector<User*> users;
    vector<pair<string, string> > pending_friend_requests;
    static World* instance;
    int main_socketfd;

    pthread_mutex_t users_mutex;
    World();

public:
    void init();
    static World *getWorld();
    void addUser(User *user);
    void removeUser(int socketid);
    bool hasUser(string username);
    string getAllUserString();
    bool handleSendMessage(User *sender, string srecv);
    void sendConfirmRequest(string target_username, string from_username);
    void refusedFriendRequest(string target_username, string from_username);
    void addFriendPair(string u1, string u2);
    void syncWithAllExcept(int exceptId);
};

#endif

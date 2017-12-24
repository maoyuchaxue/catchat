#ifndef WORLD_H
#define WORLD_H

class World;

#include "User.h"
#include "FileServer.h"
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
    void addUser(User *user); // add to online list
    void removeUser(int socketid); // remove from online list
    bool hasUser(string username); // ask if in online list
    string getAllUserString(); // get formated string for all online users
    bool handleSendMessage(User *sender, string srecv); // handle a send message request
    bool handleSendFile(User *sender, string srecv); // handle a send file request
    void sendConfirmRequest(string target_username, string from_username); // send friend request to user
    void refusedFriendRequest(string target_username, string from_username); // notify the friend request is refused
    void addFriendPair(string u1, string u2); // notify the friend request is accepted
    void syncWithAllExcept(int exceptId); // notify all users when online user list changes
    void notifyFileSendingToUser(FileInfo file_info); // notify a file is sending to you
};

#endif

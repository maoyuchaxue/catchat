
#ifndef USER_H
#define USER_H

class User;

#include <vector>
#include "World.h"
#include <string.h>
#include <pthread.h>
#include <string>
#include "FriendList.h"

using namespace std;

class User {
private:
    int socketid;
    pthread_t thread_id;
    World *world;
    string username;
    bool isLogin;
    FriendList *friend_list;
    static void *run(void *user);
    static string getFirstWord(string raw_data);

public:
    User(World *_world, int _socketid); 
    void startThread();
    World *getWorld();
    int getSocketId();

    void setUsername(string _username) {username = _username;}
    string getUsername() {return username;}
    FriendList *getFriendList() {return friend_list;}
    
    string checkLogin(string srecv);
    string checkRegister(string srecv);

    // bool checkLogin(string srecv);
    // bool checkLogin(string srecv);
    // bool checkLogin(string srecv);
    
};

#endif
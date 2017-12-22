
#ifndef USER_H
#define USER_H

class User;

#include <vector>
#include "World.h"
#include <string.h>
#include <pthread.h>
#include <string>

using namespace std;

class User {
private:
    int socketid;
    pthread_t thread_id;
    World *world;
    string username;
    static void *run(void *user);

public:
    User(World *_world, int _socketid); 
    void startThread();
    World *getWorld();
    int getSocketId();

    void setUsername(string _username) {username = _username;}
    string getUsername() {return username;}
    
    string checkLogin(string srecv);
    string checkRegister(string srecv);

    
    // bool checkLogin(string srecv);
    // bool checkLogin(string srecv);
    // bool checkLogin(string srecv);
    
};

#endif
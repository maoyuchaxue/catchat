#ifndef VERIFY_H
#define VERIFY_H

#include <string>
#include <map>
#include <pthread.h>
using namespace std;

class Verify {
private:
    static map<string, string> users;
    static pthread_mutex_t mutex;

public:
    static void init();
    static void save();
    static bool exists(string username);
    static bool registerUser(string username, string password);
    static bool checkUser(string username, string password);
};

#endif
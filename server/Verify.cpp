#include "Verify.h"
#include "const.h"
#include <stdio.h>
#include <fstream>
using namespace std;

map<string, string> Verify::users;
pthread_mutex_t Verify::mutex;

void Verify::init() {
    ifstream fin(USER_IDENTIFY_PATH.c_str());

    while (!fin.eof()) {
        string username, password;
        fin >> username;
        fin >> password;
        Verify::users.insert(make_pair(username, password));
        printf("info in config: %s -> %s\n", username.c_str(), password.c_str());
    }

    fin.close();

    pthread_mutex_init(&(Verify::mutex), NULL);
};

void Verify::save() {
    ofstream fout(USER_IDENTIFY_PATH.c_str());
    pthread_mutex_lock(&(Verify::mutex));

    for (map<string, string>::iterator it = Verify::users.begin(); it != Verify::users.end(); ++it) {
        fout << it->first << "\n" << it->second << '\n';
    }
    fout.close();

    pthread_mutex_unlock(&(Verify::mutex));
}


bool Verify::exists(string username) {
    pthread_mutex_lock(&(Verify::mutex));
    bool res = (Verify::users.find(username) != Verify::users.end());
    pthread_mutex_unlock(&(Verify::mutex));
    return res;
}

bool Verify::registerUser(string username, string password) {
    pthread_mutex_lock(&(Verify::mutex));
    bool exists = (Verify::users.find(username) != Verify::users.end());
    if (exists) {
        pthread_mutex_unlock(&(Verify::mutex));
        return false;
    }
    Verify::users.insert(make_pair(username, password));
    pthread_mutex_unlock(&(Verify::mutex));
    save();

    return true;
}


bool Verify::checkUser(string username, string password) {
    pthread_mutex_lock(&(Verify::mutex));
    bool res = (Verify::users.find(username) != Verify::users.end()) &&
        (Verify::users[username] == password);
    pthread_mutex_unlock(&(Verify::mutex));
    return res;
}
#include "User.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include "const.h"
#include "Verify.h"


User::User(World *_world, int _socketid) {
    world = _world;
    socketid = _socketid;
    isLogin = false;
    friend_list = new FriendList();
}

void User::startThread() {
    pthread_create(&thread_id, NULL, &run, (void *)(this));
}

World *User::getWorld() {
    return world;
}

int User::getSocketId() {
    return socketid;
}

void* User::run(void *user) {
    User *that = (User *)user;

    int socketid = that->getSocketId();

    printf("socketid: %d\n", socketid);

    char p[13] = "hello world!";
    send(socketid, p, strlen(p), 0);

    while (1) {
        char buf[MAX_BUF_SIZE];
        memset(buf, 0, sizeof(buf));
        long recvbytes = recv(socketid, buf, MAX_BUF_SIZE, 0);
        if (recvbytes <= 0) {
            that->getWorld()->removeUser(socketid);
            pthread_exit(NULL);
            break;
        }


        if (buf[1] != ':') {
            continue;
        }

        string srecv = buf;
        srecv = srecv.substr(0, recvbytes);

        printf("get %s\n", srecv.c_str());

        string res_str;
        const char* res;
        int send_length;

        string tmp;
    
        switch (buf[0]) {
        case 'i': // login
            tmp = that->checkLogin(srecv);
            if (tmp != "") {
                if (that->getWorld()->hasUser(tmp)) {
                    res_str = "i:onlined";
                } else {
                    that->setUsername(tmp);
                    that->getWorld()->addUser(that);
                    res_str = "i:success" + tmp;
                }
            } else {
                res_str = "i:accounterror";
            }
            res = res_str.c_str();
            send_length = strlen(res);
            send(socketid, res, send_length, 0);
            break;

        case 'r': // register user
            tmp = that->checkRegister(srecv);
            if (tmp != "") {
                that->setUsername(tmp);
                that->getWorld()->addUser(that);
                res_str = "r:success" + tmp;
            } else {
                res_str = "r:error";
            }
            res = res_str.c_str();
            send_length = strlen(res);
            send(socketid, res, send_length, 0);
            break;

        case 'u': // get list of all users
            res_str = "u:" + that->getWorld()->getAllUserString();
            res = res_str.c_str();
            send_length = strlen(res);
            send(socketid, res, send_length, 0);
            break;

        case 'x': // user exits
            that->getWorld()->removeUser(socketid);
            pthread_exit(NULL);
            return NULL;

        case 'm': // send message
            if (that->getWorld()->handleSendMessage(that, srecv.substr(2))) {
                res_str = "m:success";
            } else {
                res_str = "m:error";
            }
            res = res_str.c_str();
            send_length = strlen(res);
            send(socketid, res, send_length, 0);
            break;


        case 'f': // send file
            if (that->getWorld()->handleSendFile(that, srecv.substr(2))) {
                res_str = "f:success";
            } else {
                res_str = "f:error";
            }
            printf("f result: %s\n", res_str.c_str());
            res = res_str.c_str();
            send_length = strlen(res);
            send(socketid, res, send_length, 0);
            break;

        case 'w': // get pending messages
            that->getWorld()->notifyAllPendingMessages(that->getUsername());
            break;

        case 'l': // get friend list
            res_str = "l:" + that->getFriendList()->getFriendListString();
            res = res_str.c_str();
            send_length = strlen(res);
            send(socketid, res, send_length, 0);
            break;

        case 'a': // add in friend list
            // that->getFriendList()->addFriend(getFirstWord(srecv));
            that->getWorld()->sendConfirmRequest(getFirstWord(srecv), that->getUsername());
            break;

        case 'g': // add friend confirmation
            that->getWorld()->addFriendPair(getFirstWord(srecv), that->getUsername());
            break;

        case 'p': // refuse friend confirmation
            that->getWorld()->refusedFriendRequest(that->getUsername(), getFirstWord(srecv));
            break;

        case 'd': // remove from friend list
            that->getFriendList()->removeFriend(getFirstWord(srecv));
            res_str = "l:" + that->getFriendList()->getFriendListString();
            res = res_str.c_str();
            send_length = strlen(res);
            send(socketid, res, send_length, 0);
            break;
        }
    }
}


string User::checkLogin(string srecv) {
    printf("login: %s\n", srecv.c_str());

    if (isLogin) {
        return "";
    }

    int d1 = srecv.find('|', 2);
    if (d1 == -1) {
        return "";
    }

    string username = srecv.substr(2, d1-2);

    int d2 = srecv.find('|', d1+1);
    if (d2 == -1) {
        return "";
    }

    string password = srecv.substr(d1+1, d2-d1-1);

    if (Verify::checkUser(username, password)) {
        isLogin = true;
        friend_list->loadUserData(username);
        return username;
    } else {
        return "";
    }
}

string User::checkRegister(string srecv) {
    printf("register: %s\n", srecv.c_str());

    if (isLogin) {
        return "";
    }

    int d1 = srecv.find('|', 2);
    if (d1 == -1) {
        return "";
    }

    string username = srecv.substr(2, d1-2);

    int d2 = srecv.find('|', d1+1);
    if (d2 == -1) {
        return "";
    }

    string password = srecv.substr(d1+1, d2-d1-1);

    if (Verify::registerUser(username, password)) {
        isLogin = true;
        friend_list->loadUserData(username);
        return username;
    } else {
        return "";
    }
}

string User::getFirstWord(string raw_data) {
    string de_prefixed = raw_data.substr(2);
    int ind = de_prefixed.find("|");
    if (ind < 0) {
        return "";
    } else {
        return de_prefixed.substr(0, ind);
    }
}
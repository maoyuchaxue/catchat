#include "World.h"
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "const.h"
#include "Verify.h"
#include "FileServer.h"
// #include <netdb.h>

World* World::instance = 0; 

World::World() {

};

World* World::getWorld() {
    if (instance == 0) {
        instance = new World();
    }

    return instance;
};

void World::init() {
    main_socketfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in s_addr_in;
    memset(&s_addr_in,0,sizeof(s_addr_in));
    s_addr_in.sin_family = AF_INET;
    s_addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr_in.sin_port = htons(SERVER_PORT);
    
    bind(main_socketfd, (struct sockaddr *)(&s_addr_in), sizeof(s_addr_in));
    listen(main_socketfd, 10);
    pthread_mutex_init(&users_mutex, NULL);

    Verify::init();    

    while (1) {
        printf("waiting for connection...\n");
        printf("%d online users: ", (int)users.size());

        for (int i = 0; i < users.size(); i++) {
            printf(" %s", users[i]->getUsername().c_str());
        }

        printf("\n");

        struct sockaddr_in s_addr_client;
        int client_length = sizeof(s_addr_client);
        int user_socketfd = accept(main_socketfd, (struct sockaddr *)(&s_addr_client), (socklen_t *)(&client_length));
        if (user_socketfd != -1) {
            User *new_user = new User(this, user_socketfd);
            new_user->startThread();
        }
    }
};

void World::removeUser(int socketid) {

    pthread_mutex_lock(&users_mutex);
    for (int i = 0; i < users.size(); i++) {
        if (users[i]->getSocketId() == socketid) {
            printf("user %s exits\n", users[i]->getUsername().c_str());
            users.erase(users.begin()+i);
            break;
        }
    }
    close(socketid);
    syncWithAllExcept(socketid);
    pthread_mutex_unlock(&users_mutex);
}

void World::addUser(User *user) {
    pthread_mutex_lock(&users_mutex);
    users.push_back(user);
    syncWithAllExcept(user->getSocketId());
    pthread_mutex_unlock(&users_mutex);
}

bool World::hasUser(string username) {
    pthread_mutex_lock(&users_mutex);
    for (int i = 0; i < users.size(); i++) {
        if (users[i]->getUsername() == username) {
            return true;
        }
    }
    pthread_mutex_unlock(&users_mutex);
    return false;
}

string World::getAllUserString() {
    string res_str = "";

    pthread_mutex_lock(&users_mutex);
    for (int i = 0; i < users.size(); i++) {
        res_str += users[i]->getUsername() + "|";
    }
    pthread_mutex_unlock(&users_mutex);

    return res_str;
}

bool World::handleSendMessage(User *sender, string srecv) {
    
    int d1 = srecv.find('|', 2);
    if (d1 == -1) {
        return false;
    }

    string username = srecv.substr(0, d1);
    string send_message = srecv.substr(d1+1);

    string message = "M:" + sender->getUsername() + "|" + send_message + "|";

    bool foundUser = false;
    pthread_mutex_lock(&users_mutex);
    for (int i = 0; i < users.size(); i++) {
        if (username == users[i]->getUsername()) {
            foundUser = true;
            int socketid = users[i]->getSocketId();
            send(socketid, message.c_str(), strlen(message.c_str()), 0);
            break;
        }
    }

    // if (!foundUser) {

    // }
    pthread_mutex_unlock(&users_mutex);

    return foundUser;
}


bool World::handleSendFile(User *sender, string srecv) {
    int d1 = srecv.find('|', 0);
    if (d1 == -1) {
        return false;
    }

    int d2 = srecv.find('|', d1+1);
    if (d2 == -1) {
        return false;
    }

    int d3 = srecv.find('|', d2+1);
    if (d3 == -1) {
        return false;
    }

    string target = srecv.substr(0, d1);
    string filename = srecv.substr(d1+1, d2-d1-1);
    int size = stoi(srecv.substr(d2+1, d3-d2-1));

    string from = sender->getUsername();

    FileInfo info;
    info.target = target;
    info.name = filename;
    info.size = size;
    info.from = from;
    info.isSendingIn = false;

    FileServer::getFileServer()->addPendingFileInfo(info);
    return true;
}

void World::notifyFileSendingToUser(FileInfo file_info) {
    printf("notify user that file is sent!\n");
}

void World::sendConfirmRequest(string target_username, string from_username) {
    if (target_username == from_username) {
        return ;
    }
    string message = "F:" + from_username + "|";

    bool foundUser = false;
    pthread_mutex_lock(&users_mutex);

    for (int i = 0; i < users.size(); i++) {
        if (target_username == users[i]->getUsername()) {
            foundUser = true;
            int socketid = users[i]->getSocketId();
            send(socketid, message.c_str(), strlen(message.c_str()), 0);
            break;
        }
    }

    pending_friend_requests.push_back(make_pair(target_username, from_username));

    pthread_mutex_unlock(&users_mutex); 
}

void World::addFriendPair(string u1, string u2) {

    pthread_mutex_lock(&users_mutex);
    for (int i = 0; i < users.size(); i++) {
        if (u1 == users[i]->getUsername()) {
            FriendList *friendList = users[i]->getFriendList();
            friendList->addFriend(u2);
            string message = "l:" + friendList->getFriendListString();

            int socketid = users[i]->getSocketId();
            send(socketid, message.c_str(), strlen(message.c_str()), 0);
        }

        if (u2 == users[i]->getUsername()) {
            FriendList *friendList = users[i]->getFriendList();
            friendList->addFriend(u1);
            string message = "l:" + friendList->getFriendListString();
            
            int socketid = users[i]->getSocketId();
            send(socketid, message.c_str(), strlen(message.c_str()), 0);
        }
    }

    vector<int> delete_indexes;

    for (int i = 0; i < pending_friend_requests.size(); i++) {
        pair<string, string> req = pending_friend_requests[i];
        if ((req.first == u1 && req.second == u2) && (req.first == u2 && req.second == u1)) {
            // is a pending request
            delete_indexes.push_back(i);
        }
    }

    for (int i = delete_indexes.size()-1; i >= 0; i--) {
        pending_friend_requests.erase(pending_friend_requests.begin() + i);
    }

    pthread_mutex_unlock(&users_mutex); 
}

void World::refusedFriendRequest(string target_username, string from_username) {

    pthread_mutex_lock(&users_mutex);
    vector<int> delete_indexes;

    for (int i = 0; i < pending_friend_requests.size(); i++) {
        pair<string, string> req = pending_friend_requests[i];
        if (req.first == target_username && req.second == from_username) {
            // is a pending request
            delete_indexes.push_back(i);
        }
    }

    for (int i = delete_indexes.size()-1; i >= 0; i--) {
        pending_friend_requests.erase(pending_friend_requests.begin() + i);
    }

    pthread_mutex_unlock(&users_mutex); 
}


void World::syncWithAllExcept(int exceptId) {
    string res_str = "u:";
    for (int i = 0; i < users.size(); i++) {
        res_str += users[i]->getUsername() + "|";
    }

    printf("sync: %s\n", res_str.c_str());

    for (int i = 0; i < users.size(); i++) {
        int tid = users[i]->getSocketId();
        if (tid != exceptId) {
            send(tid, res_str.c_str(), strlen(res_str.c_str()), 0);
        }
    }
}
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
    pthread_mutex_unlock(&users_mutex);
}

void World::addUser(User *user) {
    pthread_mutex_lock(&users_mutex);
    users.push_back(user);
    pthread_mutex_unlock(&users_mutex);
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
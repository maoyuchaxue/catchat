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

    while (1) {
        printf("waiting for connection...\n");
        struct sockaddr_in s_addr_client;
        int client_length = sizeof(s_addr_client);
        int user_socketfd = accept(main_socketfd, (struct sockaddr *)(&s_addr_client), (socklen_t *)(&client_length));
        if (user_socketfd != -1) {
            User *new_user = new User(this, user_socketfd);
            users.push_back(new_user);
        }
    }
};
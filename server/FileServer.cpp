#include "FileServer.h"
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
#include "World.h"
#include <fstream>

using namespace std;


FileServer* FileServer::instance = 0;

FileServer* FileServer::getFileServer() {
    if (instance == 0) {
        instance = new FileServer();
        instance->init();
    }

    return instance;
}


void FileServer::init() {
    file_socketfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in s_addr_in;
    memset(&s_addr_in,0,sizeof(s_addr_in));
    s_addr_in.sin_family = AF_INET;
    s_addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr_in.sin_port = htons(FILE_SERVER_PORT);
    
    bind(file_socketfd, (struct sockaddr *)(&s_addr_in), sizeof(s_addr_in));
    listen(file_socketfd, 10);
    pthread_mutex_init(&files_mutex, NULL);

    pthread_create(&thread_id, NULL, &run, (void *)(this));
};

void *FileServer::run(void *file_server) {
    FileServer *that = (FileServer *)file_server;
    while (1) {
        printf("waiting for file connection...\n");

        struct sockaddr_in s_addr_client;
        int client_length = sizeof(s_addr_client);
        int sub_socketfd = accept(that->getFileSocketFd(), (struct sockaddr *)(&s_addr_client), (socklen_t *)(&client_length));
        if (sub_socketfd != -1) {
            FileThread *new_thread = new FileThread(that, sub_socketfd);
            that->addFileThread(new_thread);
            new_thread->startThread();
        }
    }
}

bool FileServer::hasPendingFileInfo(FileInfo info) {
    pthread_mutex_lock(&files_mutex);
    for (int i = 0; i < file_infos.size(); i++) {
        if (info == file_infos[i]) {
            pthread_mutex_unlock(&files_mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&files_mutex);
    return false;
}

bool FileServer::hasSentPendingFileInfo(FileInfo info) {
    pthread_mutex_lock(&files_mutex);
    for (int i = 0; i < file_infos.size(); i++) {
        if (info == file_infos[i] && file_infos[i].isSent) {
            pthread_mutex_unlock(&files_mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&files_mutex);
    return false;
}


void FileServer::addPendingFileInfo(FileInfo info) {
    pthread_mutex_lock(&files_mutex);
    file_infos.push_back(info);
    printf("adding pending file: %s %s %s %d\n", info.target.c_str(), info.from.c_str(), info.name.c_str(), info.size);
    pthread_mutex_unlock(&files_mutex);
}


void FileServer::removePendingFileInfo(FileInfo remove_info) {
    pthread_mutex_lock(&files_mutex);
    for (int i = 0; i < file_infos.size(); i++) {
        if (remove_info == file_infos[i]) {
            file_infos.erase(file_infos.begin() + i);
            break;
        }
    }
    pthread_mutex_unlock(&files_mutex);
}


void FileServer::setPendingFileInfoSent(FileInfo modify_info) {
    pthread_mutex_lock(&files_mutex);
    for (int i = 0; i < file_infos.size(); i++) {
        if (modify_info == file_infos[i]) {
            file_infos[i].isSent = true;
            break;
        }
    }
    pthread_mutex_unlock(&files_mutex);
}

void FileServer::setPendingFileInfoSending(FileInfo modify_info) {
    pthread_mutex_lock(&files_mutex);
    for (int i = 0; i < file_infos.size(); i++) {
        if (modify_info == file_infos[i]) {
            file_infos[i].isSendingIn = true;
            break;
        }
    }
    pthread_mutex_unlock(&files_mutex);
}

void FileServer::addFileThread(FileThread *thread) {
    pthread_mutex_lock(&files_mutex);
    file_threads.push_back(thread);
    pthread_mutex_unlock(&files_mutex);
}

void FileServer::removeFileThread(FileThread *thread) {
    pthread_mutex_lock(&files_mutex);
    for (int i = 0; i < file_threads.size(); i++) {
        if (file_threads[i] == thread) {
            close(file_threads[i]->getSocketId()); 
            file_threads.erase(file_threads.begin() + i);
            break;
        }
    }
    pthread_mutex_unlock(&files_mutex);
}


FileThread::FileThread(FileServer* _file_server, int _socketid) {
    file_server = _file_server;
    socketid = _socketid;
    has_file_info = false;
}

void FileThread::startThread() {
    pthread_create(&thread_id, NULL, &run, (void *)(this));
}

int FileThread::getSocketId() {
    return socketid;
}


void* FileThread::run(void *file_socket) {
    FileThread *that = (FileThread *) file_socket;
    int socketid = that->getSocketId();
    printf("file socketid: %d\n", socketid);

    bool isSend = false;

    while (1) {
        char buf[MAX_BUF_SIZE];
        memset(buf, 0, sizeof(buf));
        long recvbytes = recv(socketid, buf, MAX_BUF_SIZE, 0);

        if (recvbytes <= 0) {
            FileServer::getFileServer()->removeFileThread(that);
            pthread_exit(NULL);
            return NULL;
        }

        printf("get %s\n", buf);

        if (buf[1] != ':') {
            continue;
        }


        string srecv = buf;

        string res_str;
        const char* res;
        int send_length;

        string tmp;

        bool isConfirmed = false;

        switch (buf[0]) {
        case 'r': // wish to receive file
            if (that->handleReceiveFile(srecv.substr(2))) {
                isConfirmed = true;
                isSend = false;
            }
            break;
        case 's': // wish to send file
            if (that->handleSendFile(srecv.substr(2))) {
                isConfirmed = true;
                isSend = true;
                res_str = "ok";
                res = res_str.c_str();
                send_length = strlen(res);
                send(socketid, res, send_length, 0);
            } else {
                res_str = "bad";
                res = res_str.c_str();
                send_length = strlen(res);
                send(socketid, res, send_length, 0);
            }
            break;
        }

        if (isConfirmed) {
            break;
        }
    }

    if (isSend) {
        that->startSendFile();
    } else {
        that->startReceiveFile();
    }

    FileServer::getFileServer()->removeFileThread(that);
    return NULL;
}


bool FileThread::handleSendFile(string raw_data) {
    int d1 = raw_data.find('|', 0);
    if (d1 == -1) {
        return false;
    }

    int d2 = raw_data.find('|', d1+1);
    if (d2 == -1) {
        return false;
    }

    int d3 = raw_data.find('|', d2+1);
    if (d3 == -1) {
        return false;
    }

    int d4 = raw_data.find('|', d3+1);
    if (d4 == -1) {
        return false;
    }

    string target = raw_data.substr(0, d1);
    string from = raw_data.substr(d1+1, d2-d1-1);
    string filename = raw_data.substr(d2+1, d3-d2-1);
    int size = stoi(raw_data.substr(d3+1, d4-d3-1));

    printf("send file: %s %s %s %d\n", target.c_str(), from.c_str(), filename.c_str(), size);

    FileInfo info;
    info.target = target;
    info.name = filename;
    info.size = size;
    info.from = from;
    info.isSendingIn = true;
    info.isSent = false;

    file_info = info;

    if (FileServer::getFileServer()->hasPendingFileInfo(info)) {
        FileServer::getFileServer()->setPendingFileInfoSending(info);
        printf("is good\n");
        return true;
    } else {
        printf("is not good\n");
        return false;
    }


}

bool FileThread::handleReceiveFile(string raw_data) {
    int d1 = raw_data.find('|', 0);
    if (d1 == -1) {
        return false;
    }

    int d2 = raw_data.find('|', d1+1);
    if (d2 == -1) {
        return false;
    }

    int d3 = raw_data.find('|', d2+1);
    if (d3 == -1) {
        return false;
    }

    int d4 = raw_data.find('|', d3+1);
    if (d4 == -1) {
        return false;
    }
    

    string target = raw_data.substr(0, d1);
    string from = raw_data.substr(d1+1, d2-d1-1);
    string filename = raw_data.substr(d2+1, d3-d2-1);
    int size = stoi(raw_data.substr(d3+1, d4-d3-1));

    printf("recv file: %s %s %s %d\n", target.c_str(), from.c_str(), filename.c_str(), size);

    FileInfo info;
    info.target = target;
    info.name = filename;
    info.size = size;
    info.from = from;
    info.isSendingIn = true;
    info.isSent = true;

    file_info = info;

    if (FileServer::getFileServer()->hasSentPendingFileInfo(info)) {
        FileServer::getFileServer()->removePendingFileInfo(info);
        return true;
    } else {
        return false;
    }
}

void FileThread::startSendFile() {

    ofstream fout;

    string local_file_addr = FileServer::getLocalFileAddr(file_info);
    fout.open(local_file_addr, ios::out | ios::binary);

    int total_recv = 0;
    while (1) {
        char buf[MAX_FILE_BUF_SIZE];
        memset(buf, 0, sizeof(buf));
        long recvbytes = recv(socketid, buf, MAX_BUF_SIZE, 0);

        if (recvbytes <= 0) {
            pthread_exit(NULL);
            break;
        }

        total_recv += recvbytes;
        printf("get %ld file data, total %d\n", recvbytes, total_recv);

        if (total_recv >= file_info.size) {
            fout.write(buf, - file_info.size + recvbytes + total_recv);
            break;
        } else {
            fout.write(buf, recvbytes);
        }
    }

    fout.close();

    FileServer::getFileServer()->setPendingFileInfoSent(file_info);
    World::getWorld()->notifyFileSendingToUser(file_info);
}

void FileThread::startReceiveFile() {


}
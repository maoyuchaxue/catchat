#ifndef FILESERVER_H
#define FILESERVER_H

struct FileInfo;
class FileServer;
class FileThread;

#include <string>
#include <vector>
#include <pthread.h>
#include "const.h"

using namespace std;

struct FileInfo {
    string from;
    string target;
    int size;
    string name;
    bool isSendingIn;
    bool isSent;

    bool operator==(const FileInfo &info_b) {
        return (from == info_b.from && target == info_b.target
             && size == info_b.size && name == info_b.name);
    }
};

class FileServer {
private:
    static FileServer* instance;
    int file_socketfd;
    pthread_mutex_t files_mutex;
    pthread_t thread_id;
    FileServer(){};

    static void *run(void *file_server);

    vector<FileInfo> file_infos;
    vector<FileThread*> file_threads;

public:
    void init();
    int getFileSocketFd() {return file_socketfd;}
    static FileServer *getFileServer();
    void addPendingFileInfo(FileInfo info);
    void addFileThread(FileThread *thread);
    void removeFileThread(FileThread *thread);
    void removePendingFileInfo(FileInfo remove_info);
    void setPendingFileInfoSent(FileInfo modify_info);
    void setPendingFileInfoSending(FileInfo modify_info);
    bool hasPendingFileInfo(FileInfo info);
    bool hasSentPendingFileInfo(FileInfo info);

    static string getLocalFileAddr(FileInfo info) {
        return USER_FILE_ROOTPATH + info.from + "_" + info.target + "_" + to_string(info.size) + "_" + info.name;
    };
    // FileInfo getFileInfo();

};

class FileThread {
private:
    int socketid;
    pthread_t thread_id;
    string username;
    FileInfo file_info;
    bool has_file_info;
    FileServer* file_server;
    static void *run(void *file_socket);

public:
    FileThread(FileServer* _file_server, int _socketid);
    void startThread();
    int getSocketId();

    void setFileInfo(FileInfo _file_info) {file_info = _file_info;}
    FileInfo getFileInfo() {return file_info;}    

    bool handleSendFile(string raw_data);
    bool handleReceiveFile(string raw_data);
    void startSendFile();
    void startReceiveFile();
};

#endif

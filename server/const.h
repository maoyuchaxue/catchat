#ifndef CONST_H
#define CONST_H

#include <string>

using namespace std;

const int SERVER_PORT = 4396;
const int FILE_SERVER_PORT = 4395;
const int MAX_BUF_SIZE = 1024;
const int MAX_FILE_BUF_SIZE = 10 * 1024;

const string USER_IDENTIFY_PATH = "./data/ident.conf";
const string USER_FRIENDLIST_ROOTPATH = "./data/friendlist/";
const string USER_LATESTMESSAGE_ROOTPATH = "./data/message/";
const string USER_FILE_ROOTPATH = "./data/files/";

#endif
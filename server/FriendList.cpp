#include "FriendList.h"
#include <string>
#include "const.h"
#include <fstream>
#include <algorithm>
using namespace std;


FriendList::FriendList() {
    cur_username = "";
}

void FriendList::loadUserData(string name) {
    string file_path = USER_FRIENDLIST_ROOTPATH + name;
    
    cur_username = name;
    usernames.clear();

    ifstream fin(file_path.c_str());
    
    while (fin) {
        string nm;
        fin >> nm;
        usernames.push_back(nm);
    }

    fin.close();

    save();
}

string FriendList::getFriendListString() {
    string res_str = "";
    for (int i = 0; i < usernames.size(); i++) {
        res_str += usernames[i] + "|";
    }
    return res_str;
}

void FriendList::addFriend(string name) {
    if (cur_username == "") return ;

    if (!isFriend(name)) {
        usernames.push_back(name);
        save();
    }
}

void FriendList::removeFriend(string name) {
    if (cur_username == "") return ;

    vector<string>::iterator ind = find(usernames.begin(), usernames.end(), name);
    if (ind != usernames.end()) {
        usernames.erase(ind);
        save();
    }
}

bool FriendList::isFriend(string name) {
    if (cur_username == "") return false;

    vector<string>::iterator ind = find(usernames.begin(), usernames.end(), name);
    return (ind != usernames.end());
}


void FriendList::save() {
    string file_path = USER_FRIENDLIST_ROOTPATH + cur_username;

    ofstream fout(file_path.c_str());
    for (int i = 0; i < usernames.size(); i++) {
        fout << usernames[i] << endl;
    }
    fout.close();
}
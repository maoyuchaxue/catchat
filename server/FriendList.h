#ifndef FRIENDLIST_H
#define FRIENDLIST_H

class FriendList;

#include <string>
#include <vector>

using namespace std;

class FriendList {
private:
    vector<string> usernames;
    string cur_username;
    void save();
public:
    FriendList();
    void loadUserData(string name);
    string getFriendListString();
    void addFriend(string name);
    void removeFriend(string name);
    bool isFriend(string name);
};

#endif
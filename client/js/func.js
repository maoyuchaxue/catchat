
const { ipcRenderer } = require('electron');


var net = require('net');

var HOST = '127.0.0.1';
var PORT = 4396;

var logged_in = false;
var global_username = null;
var chating_with_username = null;
var pending_request_username = null;
var global_search_word = "";

var allusers = [];
var tot_users = 0;
var allfriends = [];
var tot_friends = 0;
var pendingusers = [];

var all_messages_with_user = {};

var global_initing = false;

var client = new net.Socket();
client.connect(PORT, HOST, function() {

    console.log('CONNECTED TO: ' + HOST + ':' + PORT);
    // 建立连接后立即向服务器发送数据，服务器将收到这些数据 

    // client.write("u:");
});

// 为客户端添加“data”事件处理函数
// data是服务器发回的数据
client.on('data', function(data) {

    console.log('DATA: ' + data);
    // // 完全关闭连接
    // client.destroy();
    dstr = data.toString();

    if (dstr.length < 2 || dstr[1] != ":") {
        return ;
    }

    switch (dstr[0]) {
        case 'i': 
            checkLoginReturn(dstr.substring(2));
            break;
        case 'r': 
            checkRegisterReturn(dstr.substring(2));
            break;
        case 'f':
            getSendFileOK(dstr.substring(2));
            break;
        case 'u':
            checkAllUsers(dstr.substring(2));
            break;
        case 'l':
            checkAllFriends(dstr.substring(2));
            break;
        case 'M':
            getChatMessage(dstr.substring(2));
            break;
        case 'F':
            getFriendRequest(dstr.substring(2));
            break;
        case 'W':
            getIncomingFile(dstr.substring(2));
            break;
    }
    
});


// 为客户端添加“close”事件处理函数
client.on('close', function() {
    console.log('Connection closed');
});

$("#login-modal").modal();
$("#error-modal").hide();


$("#login-button").click(function() {
    var urnm = $("#input-username").val();
    var pswd = $("#input-password").val();
    if (urnm.indexOf("|") >= 0 || pswd.indexOf("|") >= 0 || urnm.indexOf(":") >= 0 || pswd.indexOf(":") >= 0) {
        $("#error-modal").text("用户名、密码不能含有|, :特殊字符（用于协议）");
        $("#error-modal").show();
    } else {
        if (urnm.length < 4 || pswd.length < 4) {
            $("#error-modal").text("用户名、密码长度不应少于4位");
            $("#error-modal").show();
        } else {
            client.write("i:" + urnm + "|" + pswd + "|");
        }
    }
});


$("#register-button").click(function() {
    var urnm = $("#input-username").val();
    var pswd = $("#input-password").val();
    if (urnm.indexOf("|") >= 0 || pswd.indexOf("|") >= 0 || urnm.indexOf(":") >= 0 || pswd.indexOf(":") >= 0) {
        $("#error-modal").text("用户名、密码不能含有|, :特殊字符（用于协议）");
        $("#error-modal").show();
    } else {
        if (urnm.length < 4 || pswd.length < 4) {
            $("#error-modal").text("用户名、密码长度不应少于4位");
            $("#error-modal").show();
        } else {
            client.write("r:" + urnm + "|" + pswd + "|");
        }
    }
});


function checkLoginReturn(retStr) {
    if (retStr.substring(0, 7) == "success") {
        $("#error-modal").hide();
        logged_in = true;
        global_username = retStr.substring(7);
        console.log(global_username + " logged in!");
        $("#login-modal").modal('hide');
        initMain();
    } else if (retStr == "onlined") {
        $("#error-modal").text("用户已在线！");
        $("#error-modal").show();
    } else if (retStr == "accounterror") {
        $("#error-modal").text("用户名或密码错误！");
        $("#error-modal").show();
    }
}


function checkRegisterReturn(retStr) {
    if (retStr.substring(0, 7) == "success") {
        $("#error-modal").hide();
        logged_in = true;
        global_username = retStr.substring(7);
        console.log(global_username + " logged in!");
        $("#login-modal").modal('hide');
        initMain();

    } else if (retStr == "error") {
        $("#error-modal").text("用户名已被占用！");
        $("#error-modal").show();
    }
}

function initMain() {
    $("#navbar-title").text("欢迎来到Catchat, " + global_username + "!");
    global_initing = true;
    client.write("u:"); // ask for all users
}

function checkAllUsers(retStr) {
    allusers = retStr.split('|');
    tot_users = allusers.length-1;
    $("#online-users").text(tot_users);

    refreshAllUserList();
    client.write("l:");
}

function refreshAllUserList() {
    $("#user-list").empty();

    for (var i = 0; i < allusers.length-1; i++) {
        if (allusers[i].toLowerCase().indexOf(global_search_word) >= 0) {
            $("#user-list").append($('<div class="user-card online-user"></div>').text(allusers[i]));
            console.log("user: " + allusers[i]);
        }
    }
    
    $(".online-user").click(function() {
        setChatWith($(this).text());
    })
}


function checkAllFriends(retStr) {
    allfriends = retStr.split('|');
    tot_friends = allfriends.length-1;
    $("#friend-users").text(tot_friends);

    refreshFriendList();
    if (global_initing) {
        global_initing = false;
        client.write("w:");
    }
}

function refreshFriendList() {

    $("#friend-list").empty();

    for (var i = 0; i < allfriends.length-1; i++) {
        if (allfriends[i].toLowerCase().indexOf(global_search_word.toLowerCase()) >= 0) {
            $("#friend-list").append($('<div class="user-card friend-user"></div>').text(allfriends[i]));
            console.log("friend: " + allfriends[i]);
        }
    }

    $(".friend-user").click(function() {
        setChatWith($(this).text());
    })
}

function setChatWith(username) {
    chating_with_username = username;
    $("#chat-title-text").text(username);
    $("#chat-message").empty();

    if (allfriends.indexOf(chating_with_username) < 0) {
        $("#friend-operation-button").text("加为好友");
        $("#input-message-group").hide();
        return ;
    }

    $("#friend-operation-button").text("删除好友");

    messages = all_messages_with_user[username];

    if (messages) {
        for (var i = 0; i < messages.length; i++) {
            message = messages[i];
            if (message.side == 0) {
                if (message.text) {
                    var t_message = $('<div class="chat-cur-user"></div>');
                    t_message.append($('<span class="message-cur-user"></span>').text(decodeURI(message.text)));
                    $("#chat-message").append(t_message);
                } else {
                    var t_message = $('<div class="chat-cur-user"></div>');
                    var t_text = $('<div class="message-cur-user"></div>');
                    t_text.append($('<span class="file-cur-user"></span>').text("文件：" + message.file.file_name + " " + message.file.file_size + "字节"));
                    t_message.append(t_text);
                    $("#chat-message").append(t_message);
                }

            } else {
                if (message.text) {
                    var t_message = $('<div class="chat-other-user"></div>');
                    t_message.append($('<span class="message-other-user"></span>').text(decodeURI(message.text)));
                    $("#chat-message").append(t_message);
                } else {

                    var t_message = $('<div class="chat-other-user"></div>');
                    var t_text = $('<div class="message-other-user"></div>');
                    t_text.append($('<span class="file-other-user"></span>').text("文件：" + message.file.file_name + " " + message.file.file_size + "字节"));
                    var download_button = null;

                    if (message.file.downloaded) {
                        download_button = $('<span class="file-download-button"></span>').text("下载完毕");
                    } else {

                        download_button = $('<span class="file-download-button"></span>').text("下载");
                        var onDownload = function() {
                            console.log("start download");
                            ipcRenderer.sendSync('download-file', {
                                target: global_username,
                                from: chating_with_username,
                                filename: message.file.file_name,
                                size: message.file.file_size
                            });
                        };

                        $(download_button).click(onDownload);
                    }

                    t_text.append(download_button);
                    t_message.append(t_text);
                    $("#chat-message").append(t_message);
                }
            }
        }
    }

    $("#input-message-group").show();
    $("#toggle-friend-button").show();
}

function sendChatMessage() {
    if (chating_with_username == null) {
        return ;
    }

    messages = all_messages_with_user[chating_with_username];
    var cur_message = $("#input-message").val();
    if (cur_message.length <= 0) {
        return ;
    }

    if (messages) {
        all_messages_with_user[chating_with_username].push({
            "side" : 0,
            "text" : cur_message,
            "file" : null
        });
    } else {
        all_messages_with_user[chating_with_username] = [{
            "side" : 0,
            "text" : cur_message,
            "file" : null
        }];
    }

    var t_message = $('<div class="chat-cur-user"></div>');
    t_message.append($('<span class="message-cur-user"></span>').text(cur_message));
    $("#chat-message").append(t_message);
    client.write("m:" + chating_with_username + "|" + encodeURI(cur_message) + "|");
    $("#input-message").val("");
}

function getChatMessage(recvStr) {
    infos = recvStr.split('|');
    if (infos.length < 2) {
        return ;
    }

    from_user = infos[0];
    message = infos[1];

    messages = all_messages_with_user[from_user];

    if (messages) {
        all_messages_with_user[from_user].push({
            "side" : 1,
            "text" : message,
            "file" : null
        });
    } else {
        all_messages_with_user[from_user] = [{
            "side" : 1,
            "text" : message,
            "file" : null
        }];
    }

    if (from_user == chating_with_username) {
        var t_message = $('<div class="chat-other-user"></div>');
        t_message.append($('<span class="message-other-user"></span>').text(decodeURI(message)));
        $("#chat-message").append(t_message);
    }
}

function toggleFriend() {
    if (allfriends.indexOf(chating_with_username) < 0) {
        // not friend
        client.write("a:" + chating_with_username + "|");
    } else {
        client.write("d:" + chating_with_username + "|");
    }
}

function getFriendRequest(reqStr) {
    var ind = reqStr.indexOf("|");
    if (ind >= 0) {
        pendingusers.push(reqStr.substring(0, ind));
        refreshFriendRequests();
    }
}

function getIncomingFile(recvStr) {
    infos = recvStr.split('|');
    if (infos.length < 3) {
        return ;
    }

    var from_user = infos[0];
    var file_name = infos[1];
    var file_size = parseInt(infos[2]);


    messages = all_messages_with_user[from_user];

    if (messages) {
        all_messages_with_user[from_user].push({
            "side" : 1,
            "text" : null,
            "file" : {
                "file_name" : file_name,
                "file_size" : file_size,
                "downloaded" : false
            }
        });
    } else {
        all_messages_with_user[from_user] = [{
            "side" : 1,
            "text" : null,
            "file" : {
                "file_name" : file_name,
                "file_size" : file_size,
                "downloaded" : false
            }
        }];
    }

    if (from_user == chating_with_username) {
        var t_message = $('<div class="chat-other-user"></div>');
        var t_text = $('<div class="message-other-user"></div>');
        t_text.append($('<span class="file-other-user"></span>').text("文件：" + file_name + " " + file_size + "字节"));
        var download_button = $('<span class="file-download-button"></span>').text("下载");

        var onDownload = function() {
            console.log("start download");
            ipcRenderer.sendSync('download-file', {
                target: global_username,
                from: chating_with_username,
                filename: file_name,
                size: file_size
            });
        };

        $(download_button).click(onDownload);
        t_text.append(download_button);
        t_message.append(t_text);
        $("#chat-message").append(t_message);
    }
}

function acceptFriendRequest() {
    client.write("g:" + pending_request_username + "|");
    var index = pendingusers.indexOf(pending_request_username);
    if (index >= 0) {
        pendingusers.splice(index, 1);
        refreshFriendRequests();
    }
    $("#confirm-modal").modal('hide');
}

function refuseFriendRequest() {
    client.write("p:" + pending_request_username + "|");
    var index = pendingusers.indexOf(pending_request_username);
    if (index >= 0) {
        pendingusers.splice(index, 1);
        refreshFriendRequests();
    }
    $("#confirm-modal").modal('hide');
}


function refreshFriendRequests() {

    var tot_pending = pendingusers.length;
    $("#pending-users").text(tot_pending);
    $("#pending-list").empty();

    for (var i = 0; i < pendingusers.length; i++) {
        $("#pending-list").append($('<div class="user-card pending-user"></div>').text(pendingusers[i]));
        console.log("pending: " + pendingusers[i]);
    }

    $(".pending-user").click(function() {
        openPendingInfo($(this).text());
    })
}

function openPendingInfo(name) {
    pending_request_username = name;
    $("#request-friend-name").text(name);
    $("#confirm-modal").modal();
}

function openFileModal() {
    $("#file-modal").modal();
}

var global_file = null;

function dropFileHandler(e) {
    e.preventDefault();
    e.stopPropagation();

    for (let f of e.dataTransfer.files) {
        console.log("dragging in file " + f.path);
        global_file = f;
        if (f.name.indexOf('|') >= 0) {
            $("#file-dropper").text("错误：文件名中不能含有|,:特殊字符");
            break;
        }

        var fsize = ipcRenderer.sendSync('file-size', f.path);
        if (fsize == 0) {
            $("#file-dropper").text("错误：无效文件或空文件");
            break;
        }

        messages = all_messages_with_user[chating_with_username];

        if (messages) {
            all_messages_with_user[chating_with_username].push({
                "side" : 0,
                "text" : null,
                "file" : {
                    "file_name" : f.name,
                    "file_size" : fsize,
                    "downloaded" : false
                }
            });
        } else {
            all_messages_with_user[chating_with_username] = [{
                "side" : 0,
                "text" : null,
                "file" : {
                    "file_name" : f.name,
                    "file_size" : fsize,
                    "downloaded" : false
                }
            }];
        }

        var t_message = $('<div class="chat-cur-user"></div>');
        var t_text = $('<div class="message-cur-user"></div>');
        t_text.append($('<span class="file-cur-user"></span>').text("文件：" + f.name + " " + fsize + "字节"));
        t_message.append(t_text);
        $("#chat-message").append(t_message);

        client.write("f:" + chating_with_username + "|" + f.name + "|" + fsize + "|");

        break; // only one file allowed
    }

}

function getSendFileOK(recvStr) {
    if (recvStr != "success") {
        $("#file-dropper").text("错误：不明错误……");
        return ;
    }

    var res = ipcRenderer.sendSync('file-send', {
        name : global_username,
        path : global_file.path,
        target : chating_with_username,
        filename : global_file.name
    });

    if (res != "success") {
        $("#file-dropper").text(res);
    } else {
        $("#file-modal").modal('hide');

    }
}


document.getElementById("file-dropper").addEventListener("drop", dropFileHandler);
document.getElementById("file-dropper").addEventListener("dragover", function(e) {
    e.preventDefault();
    e.stopPropagation();
});



$("#send-message-button").click(sendChatMessage);
$("#input-message-group").hide();
$("#toggle-friend-button").hide();
$("#toggle-friend-button").click(toggleFriend);
$("#confirm-friend-button").click(acceptFriendRequest);
$("#refuse-friend-button").click(refuseFriendRequest);
$("#friend-operation-button").click(toggleFriend);
$("#open-file-button").click(openFileModal);

$("#file-modal").modal('hide');
$("#download-finish-modal").modal('hide');

var global_file_name = null;

ipcRenderer.on('file-download-finished', (event, arg) => {
    global_file_name = arg.name;
    $("#download-file-name").text(arg.name);
    $("#download-file-size").text(arg.size);
    $("#download-finish-modal").modal();
    // TODO
});

$("#close-download-modal-button").click(function() {
    $("#download-finish-modal").modal('hide');
});

$('#open-file-path-button').click(function() {
    $("#download-finish-modal").modal('hide');
    ipcRenderer.send('open-download-path', global_file_name);
});

$("#input-search-user").change(function () {
    global_search_word = $(this).val();
    refreshAllUserList();
    refreshFriendList();
})
var net = require('net');
var readline = require('readline');

var HOST = '127.0.0.1';
var PORT = 4396;

var client = new net.Socket();
client.connect(PORT, HOST, function() {

    console.log('CONNECTED TO: ' + HOST + ':' + PORT);
    // 建立连接后立即向服务器发送数据，服务器将收到这些数据 

    client.write("u:");
});

// 为客户端添加“data”事件处理函数
// data是服务器发回的数据
client.on('data', function(data) {

    console.log('DATA: ' + data);
    // // 完全关闭连接
    // client.destroy();
    
});

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
})

rl.on('line', function(line) {
    console.log("send " + line);
    client.write(line);
})


// 为客户端添加“close”事件处理函数
client.on('close', function() {
    console.log('Connection closed');
});
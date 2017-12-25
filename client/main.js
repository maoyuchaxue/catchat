
const {app, BrowserWindow, ipcMain, webContents, shell} = require('electron')
const path = require('path')
const url = require('url')
const fs = require('fs')
const net = require('net')

var FILE_HOST = '127.0.0.1';
var FILE_PORT = 4395;

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let win

function createWindow () {
  // Create the browser window.
  win = new BrowserWindow({width: 800, height: 600})

  // and load the index.html of the app.
  win.loadURL(url.format({
    pathname: path.join(__dirname, 'index.html'),
    protocol: 'file:',
    slashes: true
  }))

  // Open the DevTools.
  win.webContents.openDevTools()

  // Emitted when the window is closed.
  win.on('closed', () => {
    // Dereference the window object, usually you would store windows
    // in an array if your app supports multi windows, this is the time
    // when you should delete the corresponding element.
    win = null
  })
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)

// Quit when all windows are closed.
app.on('window-all-closed', () => {
  // On macOS it is common for applications and their menu bar
  // to stay active until the user quits explicitly with Cmd + Q
  if (process.platform !== 'darwin') {
    app.quit()
  }
})

app.on('activate', () => {
  // On macOS it's common to re-create a window in the app when the
  // dock icon is clicked and there are no other windows open.
  if (win === null) {
    createWindow()
  }
})

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.

ipcMain.on('file-size', (event, arg) => {
  var path = arg;

  var stats = fs.statSync(path);
  if (stats.isFile()) {
    event.returnValue = stats.size;
  } else {
    event.returnValue = 0;
  }
})

ipcMain.on('file-send', (event, arg) => {
  var name = arg.name;
  var path = arg.path;
  var filename = arg.filename;
  var target = arg.target;

  var stats = fs.statSync(path);
  if (stats.isFile()) {
    if (stats.size < 10*1024*1024) {
      startSendFile(name, path, target, filename, stats.size);
      event.returnValue = "success";
    } else {
      event.returnValue = "错误：文件过大，无法发送！";
    }
  } else {
    event.returnValue = "错误：非法文件！";
  }
  
})

function startSendFile(name, path, target, filename, size) {
  console.log("start sending file " + name + path + target);

  var client = new net.Socket();
  client.connect(FILE_PORT, FILE_HOST, function() {

      console.log('file socket linked: ' + FILE_HOST + ':' + FILE_PORT);
      // 建立连接后立即向服务器发送数据，服务器将收到这些数据 
      console.log("s:" + target + "|" + name + "|" + filename + "|" + size  + "|");
      client.write("s:" + target + "|" + name + "|" + filename + "|" + size  + "|");
  });

  client.on('data', function(data) {
    console.log(data);
    if (data.toString().substring(0,2) == "ok") {
      fs.open(path, "r", function (err, fd) {
        var buf = new Buffer(512);
        var sendNextPatch = function (fd) {
          fs.read(fd, buf, 0, 512, null, function (err, bytesRead, buffer) {
            console.log("bytesread: " + bytesRead);
            if (bytesRead > 0) {
              client.write(buffer.slice(0, bytesRead), function(err) {
                if (err) {
                  return ;
                } else {
                  sendNextPatch(fd);
                }
              });
            }
          })
        };
        sendNextPatch(fd);
      })
    }
  })

}

ipcMain.on('open-download-path', (event, arg) => {
  var path = __dirname  + "/download/" + arg;
  shell.showItemInFolder(path);
});

ipcMain.on('download-file', (event, arg) => {
  var from = arg.from;
  var target = arg.target;
  var filename = arg.filename;
  var size = arg.size;


  startReceiveFile(from, target, filename, size);

  event.returnValue = true;
});

function startReceiveFile(from, target, filename, size) {

  console.log("start receive file " + from + target + filename);

  var client = new net.Socket();
  client.connect(FILE_PORT, FILE_HOST, function() {

      console.log('file socket linked: ' + FILE_HOST + ':' + FILE_PORT);
      // 建立连接后立即向服务器发送数据，服务器将收到这些数据 
      client.write("r:" + target + "|" + from + "|" + filename + "|" + size  + "|");
  });

  var path = __dirname  + "/download/" + filename;
  var fd = fs.openSync(path, "w");

  client.on('data', function(data) {
    console.log("get data: " + data.length);
    fs.writeSync(fd, data);
  })

  client.on('close', function() {
    fs.closeSync(fd);
    
    win.webContents.send('file-download-finished', {
      from: from,
      target: target,
      filename: filename,
      size: size
    });
  })

}
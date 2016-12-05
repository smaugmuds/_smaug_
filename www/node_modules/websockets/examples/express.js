/**/
var fs = require('fs'), URL = require('url'), path = require('path');
var express = require('express');

var wss = require('../lib/websockets/websockets');

var svr = express.createServer();

svr.get('*', function(req, res) {
  var url = URL.parse(req.url);
  var file = path.join(__dirname, url.pathname);
  if(!path.existsSync(file)) {
    res.statusCode = 404;
    return res.end('' + 404);
  }
  var read = fs.createReadStream(file);
  read.pipe(res);
});

var ws = wss.createServer({
  server: svr
});

ws.on('connect', function(socket) {
  socket.on('open', function() {
    console.log('open');
  });
  socket.on('error', function() {
    console.log('error');
  });
  socket.on('message', function(data) {
    console.log('receive: ' + data);
    if('message from client' === data)
      socket.send('message from server');
  });
  socket.on('close', function() {
    console.log('close');
  });

}).listen(80);

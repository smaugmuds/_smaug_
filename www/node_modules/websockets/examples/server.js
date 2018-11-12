/**/
var fs = require('fs'), URL = require('url'), path = require('path');

var svr = require('../lib/websockets/websockets');

var ws = svr.createServer(requestListener);

ws.on('connect', onconnect).listen(80);

var wss = svr.createServer({
  validateOrigin: function(head) {
    return 'node-websockets' === head['sec-websocket-origin'];
  },
  key: fs.readFileSync(path.join(__dirname, 'ssl/ryans-key.pem')),
  cert: fs.readFileSync(path.join(__dirname, 'ssl/ryans-cert.pem'))
}, requestListener);

wss.on('connect', onconnect).listen(443);

function requestListener(req, res) {
  var url = URL.parse(req.url);
  var file = path.join(__dirname, url.pathname);
  if(!path.existsSync(file)) {
    res.statusCode = 404;
    return res.end('' + 404);
  }
  var read = fs.createReadStream(file);
  read.pipe(res);
}
function onconnect(socket) {
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
}

/**/
var top = require('../lib/websockets/websockets');

var client = new top.WebSocket('wss://127.0.0.1', {
    'rejectUnauthorized': false
});

client.on("message", function(data) {
  console.log(data);
  client.close();
});
client.on('close', function() {
  console.log('close');
});
client.on('error', function(err) {
  console.log(err);
});
client.on('open', function() {
  console.log('open');
  client.send("message from client");
});

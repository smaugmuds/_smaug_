var chat = require('..');
var room = chat.room();

// create the client
var client = chat.client(room);

// listen for data in the room
room.on('message', function(msg) {
    if (msg.data) {
        console.log(msg.data);
    }
});

client.identify({ nick: 'Ted' });

client.once('ready', function() {
    client.write('hello');
});

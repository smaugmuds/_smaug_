var chat = require('..'),
    net = require('net'),
    client = chat.client(net.connect({ port: 7080 }));

// identify the client
client.identify({ nick: 'Bill' });

// handle client ready
client.once('ready', function(metadata) {
    console.log('client connected');
});

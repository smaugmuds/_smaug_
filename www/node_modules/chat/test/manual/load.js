var assert = require('assert'),
    chat = require('../../'),
    room = chat.room(),
    randomName = require('random-name'),
    uuid = require('uuid'),
    MuxDemux = require('mux-demux'),
    count = 1;

function newConnection() {  
    var connection = room.connect(),
        client = chat.client(connection, { nick: randomName().replace(/\s/g, '') });

    count += 1;

    client.on('data', function handleMessage(msg) {
        if (msg.id === connection.id && msg.data === 'hello') {
            console.log('client created and message round-tripped, total clients = ' + count);
            room.removeListener('message', handleMessage);
            newConnection();
        }
    });

    client.write('hello');
}

newConnection();
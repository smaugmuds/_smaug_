var assert = require('assert'),
    chat = require('../../'),
    centralRoom = chat.room(),
    randomName = require('random-name'),
    uuid = require('uuid'),
    MuxDemux = require('mux-demux'),
    count = 1;

function newRoom() {  
    var room = chat.room(),
        stream = room.createStream();
        connection = room.connect(),
        client = chat.client(connection, { nick: randomName().replace(/\s/g, '') });

    // pipe the room into the central room
    stream.pipe(centralRoom.createStream()).pipe(stream);

    // incremen the room count
    count += 1;

    client.on('data', function handleMessage(msg) {
        if (msg.id === connection.id && msg.data === 'hello') {
            console.log('client created and message round-tripped, total clients = ' + count);
            room.removeListener('message', handleMessage);
            newRoom();
        }
    });

    client.write('hello');
}

newRoom();
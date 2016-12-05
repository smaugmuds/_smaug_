var assert = require('assert'),
    chat = require('../../'),
    rooms = [chat.room(), chat.room()],
    streams = rooms.map(function(room) {
        return room.createStream();
    }),
    randomName = require('random-name'),
    uuid = require('uuid'),
    MuxDemux = require('mux-demux'),
    count = 1;

// join the rooms
streams[0].pipe(streams[1]).pipe(streams[0]);

function newConnection() {  
    var roomIdx = count % rooms.length,
        room = rooms[roomIdx],
        connection = room.connect(),
        client = chat.client(connection, { nick: randomName().replace(/\s/g, '') });

    count += 1;

    client.on('data', function handleMessage(msg) {
        if (msg.id === connection.id && msg.data === 'hello') {
            console.log('client created (in room: ' + roomIdx + ') and message round-tripped, total clients = ' + count);
            console.log('room 0 connections: ' + rooms[0].connections._array.length + ', room 1 connections: ' + rooms[1].connections._array.length);

            room.removeListener('message', handleMessage);
            newConnection();
       }
    });

    client.write('hello');
}

newConnection();
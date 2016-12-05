var assert = require('assert'),
    chat = require('../'),
    connections = [],
    randomName = require('random-name'),
    uuid = require('uuid'),
    client,
    room;

describe('message removal tests', function() {
    it('should be able to create a new chat room', function() {
        room = chat.room();
    });

    it('should be able to connect to the room', function() {
        connections[0] = room.connect();
    });

    it('should be able to send messages to the room', function(done) {
        client = chat.client(connections[0]);

        room.on('message', function handleMessage(msg) {
            if (msg.data === 'hello') {
                room.removeListener('message', handleMessage);
                done();
            }
        });

        client.identify({ nick: randomName().replace(/\s/g, '') });
        client.write('hello');
    });

    it('message removal should trigger a notification', function(done) {
        var firstMessage = room.messages._array[0];

        client.once('data', function(msg) {
            assert.equal(msg.type, 'MSGREMOVE');
            assert.equal(msg.time + '|' + msg.cid, firstMessage.id);

            done();
       });

        room.messages.remove(firstMessage.id);
    });
});
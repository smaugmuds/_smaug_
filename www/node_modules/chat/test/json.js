var assert = require('assert'),
    chat = require('../'),
    connections = [],
    randomName = require('random-name'),
    uuid = require('uuid'),
    room;

describe('json message passing tests', function() {
    it('should be able to create a new chat room', function() {
        room = chat.room();
    });

    it('should be able to connect to the room', function() {
        connections[0] = room.connect();
    });

    it('should be able to send messages to the room', function(done) {
        var client = chat.client(connections[0]),
            testTargetId = uuid.v4();

        room.on('message', function handleMessage(msg) {
            if (typeof msg.data == 'object') {
                // assert that we have a target id
                assert.equal(msg.data.target, testTargetId);
                room.removeListener('message', handleMessage);
                done();
            }
        });

        client.identify({ nick: randomName().replace(/\s/g, '') });
        client.write({ target: testTargetId });
    });
});
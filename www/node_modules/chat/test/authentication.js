var assert = require('assert'),
    chat = require('../'),
    connections = [],
    randomName = require('random-name'),
    uuid = require('uuid'),
    room;

describe('chat authentication tests', function() {
    it('should be able to create a new chat room', function() {
        room = chat.room();
    });

    it('should be able to connect an authentication event to the room', function() {
        room.on('authenticate', function(details, row) {
        });
    })

    it('should be able to connect to the room', function() {
        connections[0] = room.connect();
    });

    it('should not be able to send messages as the client is not authenticated', function(done) {
        var client = chat.client(connections[0]);

        function handleMessage(msg) {
            throw new Error('Received message and should not have as we have not authenticated');
        }

        room.once('message', handleMessage);

        setTimeout(function() {
            room.removeListener('message', handleMessage);
            done();
        }, 100);

        client.write('hello');
    });

    it('should be able to authenticate the user', function(done) {
        var connection = room.connections.get(connections[0].id);

        room.once('message', function(msg) {
            assert.equal(msg.type, 'USERJOIN');
            assert.equal(msg.cid, connections[0].id);

            done();
        });

        // flag as authenticated
        connection.set('authenticated', true);
    });

    it('should be able to capture messages coming via the connected stream', function(done) {
        var client = chat.client(connections[0]);

        client.once('data', function(msg) {
            assert.equal(msg.data, 'hello');
            assert.equal(msg.cid, connections[0].id);

            done();
        });

        client.write('hello');
    });
});
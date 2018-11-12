var assert = require('assert'),
    chat = require('../'),
    connections = [],
    randomName = require('random-name'),
    uuid = require('uuid'),
    room;

describe('simple chat room initialization and client tests', function() {
    it('should be able to create a new chat room', function() {
        room = chat.room();
    });

    it('should be able to connect to the room', function() {
        connections[0] = room.connect();
    });

    it('should be able to send messages to the room', function(done) {
        var client = chat.client(connections[0]);

        room.on('message', function handleMessage(msg) {
            if (msg.data === 'hello') {
                room.removeListener('message', handleMessage);
                done();
            }
        });

        client.identify({ nick: randomName().replace(/\s/g, '') });
        client.write('hello');
    });

    it('should be able to capture messages coming via the connected stream', function(done) {
        var client = chat.client(room.connect());

        client.once('data', function handleMessage(msg) {
            if (msg.data === 'hello') {
                room.removeListener('message', handleMessage);
                done();
            }
        });

        client.identify({ nick: randomName().replace(/\s/g, '') }).on('ready', function() {
            client.write('hello');
        });
    });

    it('should receive a message backlog for a new connection', function(done) {
        var client = chat.client(room.connect());

        client.identify({ nick: randomName().replace(/\s/g, '') }).on('ready', function(metadata) {
            // validate that we have a backlog
            assert(metadata.backlog);

            // validate the 2 messages are in the backlog
            assert.equal(metadata.backlog.length, 2);

            // validate both messages say "hello"
            metadata.backlog.forEach(function(msg) {
                assert.equal(msg.data, 'hello');
            });

            done();
        });
    });
});
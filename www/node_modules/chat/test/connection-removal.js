var assert = require('assert'),
    chat = require('../'),
    connections = [],
    randomName = require('random-name'),
    uuid = require('uuid'),
    room,
    clients = [];

describe('connection removal tests', function() {
    it('should be able to create a new chat room', function() {
        room = chat.room();
    });

    it('should be able to connect to the room', function() {
        connections[0] = room.connect();
    });

    it('should be able to create client 1', function(done) {
        clients[0] = chat.client(connections[0]);

        room.on('message', function handleMessage(msg) {
            if (msg.data === 'hi') {
                room.removeListener('message', handleMessage);
                done();
            }
        });

        clients[0].identify({ nick: randomName().replace(/\s/g, '') });
        clients[0].write('hi');
    });

    it('should be able to create a second connection and client', function(done) {
        connections[1] = room.connect();
        clients[1] = chat.client(connections[1]);

        room.on('message', function handleMessage(msg) {
            if (msg.data === 'ho') {
                room.removeListener('message', handleMessage);
                done();
            }
        });

        clients[1].identify({ nick: randomName().replace(/\s/g, '') });
        clients[1].write('ho');
    });

    it('should have 2 active connections in the room', function() {
        assert.equal(room.connections._array.length, 2);
    });

    it('should be able to end the first client stream', function(done) {
        var notificationCount = 0;

        function handleMessage(msg) {
            assert.equal(msg.type, 'USERLEAVE');
            assert.equal(msg.cid, clients[0].cid);

            // increment the notification count
            notificationCount += 1;
            if (notificationCount >= 2) {
                done();
            }
        }

        room.once('message', handleMessage);
        clients[1].once('data', handleMessage);
        clients[0].end();
    });

    it('should now have only 1 active connection', function() {
        assert.equal(room.connections._array.length, 1);
    });

    it('should be able to end the second client stream', function(done) {
        room.once('message', function(msg) {
            assert.equal(msg.type, 'USERLEAVE');
            assert.equal(msg.cid, clients[1].cid);
            done();
        });

        clients[1].end();
    });

    it('should now 0 active connections', function() {
        assert.equal(room.connections._array.length, 0);
    });
});
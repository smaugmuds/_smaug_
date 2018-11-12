var assert = require('assert'),
    chat = require('../'),
    connections = [],
    randomName = require('random-name'),
    uuid = require('uuid'),
    room,
    rwClient,
    roClient;

describe('readonly user tests', function() {
    it('should be able to create a new chat room', function() {
        room = chat.room();
    });

    it('should be able to connect to the room', function() {
        connections[0] = room.connect();
    });

    it('should be able to send messages', function(done) {
        rwClient = chat.client(connections[0]);

        room.on('message', function handleMessage(msg) {
            if (msg.data === 'hello') {
                room.removeListener('message', handleMessage);
                done();
            }
        });

        rwClient.identify({ nick: randomName().replace(/\s/g, '') });
        rwClient.write('hello');
    });

    it('should be able to create a readonly client', function(done) {
        roClient = chat.client(room.connect());

        roClient.identify({ nick: randomName().replace(/\s/g, '') }, 'readonly');
        roClient.on('ready', function() {
            roClient.once('data', function(msg) {
                assert.equal(msg.data, 'hi there');
                assert.equal(msg.cid, rwClient.cid);

                done();
            });

            rwClient.write('hi there');
        });
    });

    it('should not be able to send a message from the readonly client', function(done) {
        function handleData(msg) {
            throw new Error('received data, when it should have been prevented by the server');
        }

        setTimeout(function() {
            rwClient.removeListener('data', handleData);
            done();
        }, 200);

        rwClient.on('data', handleData);
        roClient.write('um, hi');
    });
});
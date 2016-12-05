var assert = require('assert'),
    chat = require('../'),
    connections = [],
    randomName = require('random-name'),
    uuid = require('uuid'),
    client,
    room;

describe('user identification tests', function() {
    it('should be able to create a new chat room', function() {
        room = chat.room();
    });

    it('should be able to connect to the room', function() {
        client = chat.client(room);
    });

    it('should be able to provide user details for the connection', function(done) {
        var nick = randomName().replace(/\s/g, '');

        client.once('data', function(msg) {
            assert.equal(msg.type, 'JOIN');
            assert(msg.user, 'No user details found');
            assert.equal(msg.user.nick, nick, 'User details did not match provided');

            done();
        });

        client.identify({ nick: nick });
    });

    it('should receive a USERJOIN event for another client joining', function(done) {
        var nick = randomName().replace(/\s/g, ''),
            client2 = chat.client(room.connect(), { nick: nick });

        // listen for the user join event 
        client.once('data', function(msg) {
            assert.equal(msg.type, 'USERJOIN');
            assert(msg.user, 'No user details found');
            assert.equal(msg.user.nick, nick, 'User details did not match provided');

            done();
        });
    });

    it('should report existing users within the room on JOINing a room', function(done) {
        var nick = randomName().replace(/\s/g, ''),
            client3 = chat.client(room.connect(), { nick: nick }),
            ids;

        client3.once('data', function(msg) {
            assert.equal(msg.type, 'JOIN');
            assert(msg.meta, 'No room metadata found in the join data');
            assert(msg.meta.connections, 'No connection list in the metadata');
            assert.equal(msg.meta.connections.length, 3, 'Should have three users in the room');

            // extract the ids from the connections
            ids = msg.meta.connections.map(function(conn) {
                return conn.id;
            });

            // ensure client 1 is found
            assert(ids.indexOf(client.cid) >= 0, 'Could not find client 1 in list of connections');
            assert(ids.indexOf(client3.cid) >= 0, 'Could not find client 3 in list of connections');

            done();
        });

        client.identify({ nick: nick });
    });

    it('should report metadata in the ready event', function(done) {
        var nick = randomName().replace(/\s/g, ''),
            client4 = chat.client(room, { nick: nick });

        client4.once('ready', function(metadata) {
            assert(metadata, 'No metadata received in the ready event');
            assert.equal(metadata.connections.length, 4, 'Did not get the expected 4 connections');

            done();
        });
    });
});
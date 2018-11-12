var assert = require('assert'),
    uuid = require('uuid'),
    chat = require('../'),
    clients = [];

describe('chat sync', function() {
    it('should be able to create one instance of chat', function() {
        clients[0] = new Chat();
    });

    it('should not be able to send a message without identification', function() {
        assert.throws(function() {
            clients[0].send('hi');
        });
    });

    it('should be able to identify outselves', function(done) {
        var testId = uuid.v4();

        clients[0].once('join', function(id) {
            assert.equal(id, testId);
            done();
        });

        clients[0].identify(testId, { nick: 'Bill' });
        assert.equal(clients[0].uid, testId, 'Chat instance uid not matching identified id');
        assert(clients[0].users.get(testId), 'User does not exist in set of users');
    });

    it('should be able to create a second client', function() {
        clients[1] = new Chat();
    });

    it('should be able to sync client 1 with client 0', function(done) {
        setTimeout(function() {
            assert(clients[1].users.get(clients[0].uid), 'Client 1 does not know about client 0 uid');
            done();
        }, 200);

        var a = clients[0].createStream();
        a.pipe(clients[1].createStream()).pipe(a);
    });

    it('should be able to identify user 2', function() {
        clients[1].identify(uuid.v4(), { nick: 'Ted' });
    });

    it('should be able to send a message from client 0 --> client 1', function(done) {
        clients[1].once('message', function(msg) {
            assert.equal(msg.data, 'hi');
            assert(msg.user, 'No user details');
            assert.equal(msg.user.nick, 'Bill');

            done();
        });

        clients[0].send('hi');
    });

    it('should be able to send a message from client 1 --> client 0', function(done) {
        clients[0].once('message', function(msg) {
            assert.equal(msg.data, 'ho');
            assert(msg.user, 'No user details');
            assert.equal(msg.user.nick, 'Ted');

            done();
        });

        clients[1].send('ho');
    });

    it('should be able to close client 1', function(done) {
        // assert that we left
        clients[0].once('leave', function(id) {
            assert.equal(id, clients[0].uid);
            done();
        });

        clients[0].close();
    });

    it('should no longer have client 0 in the client 1 users', function(done) {
        setTimeout(function() {
            assert(!clients[1].users.has(clients[0].uid), 'User not removed when client closed');
            done();
        }, 100);
    });

    /*
    it('should be able to write messages and have them reflected locally', function(done) {
        clients[0].once('update', function(update) {          
            assert.equal(update[1], 'hi');
            done();
        });

        clients[0].send('hi');
    });

    it('should be able to create another instance of chat', function() {
        clients[1] = new Scuttlechat({ uid: uuid.v4() });
    });

    it('should be able to create streams for the chat instances', function() {
        streams[0] = clients[0].createStream();
        streams[1] = clients[1].createStream();
    });

    it('should be able to sync the chat rooms', function() {
        clients[1].once('update', function(update) {
            assert.equal(update[1], 'hi');
            done();
        });

        streams[0].pipe(streams[1]).pipe(streams[0]);
    });
*/
});
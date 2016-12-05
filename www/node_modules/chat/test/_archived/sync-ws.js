var assert = require('assert'),
    debug = require('debug')('scuttlechat-test'),
    uuid = require('uuid'),
    WebSocket = require('ws'),
    WebSocketServer = require('ws').Server,
    wsstream = require('websocket-stream'),
    Scuttlechat = require('../'),
    Model = require('scuttlebutt/model'),
    clients = [],
    streams = [],
    room,
    wsserver,
    sockets = [];

describe('scuttlechat sync (via websockets)', function() {
    it('should be able to create a room instance of chat', function() {
        room = new Scuttlechat({ uid: uuid.v4() });
    });

    it('should be able to create a websocket server', function() {
        wsserver = new WebSocketServer({ port: 3002 });
        wsserver.on('connection', function(socket) {
            socket = wsstream(socket);
            socket.on('data', function(data) {
                debug('server received: ' + data);
            });

            // create the relationship between the websocket and the room
            debug('new websocket connection, piping stream into scuttlechat room');
            socket.pipe(room.createStream()).pipe(socket);
        });
    });

    /*
    it('should be able to create a websocket client', function(done) {
        sockets[0] = new WebSocket('ws://localhost:3002/');
        sockets[0].on('open', function() {
            sockets[0].close();
            done();
        });
    });
*/

    it('should be able to create one instance of chat', function() {
        clients[0] = new Scuttlechat({ uid: uuid.v4() });
        streams[0] = clients[0].createStream();
    });

    it('should be able to wire in client 1 to the websocket server', function(done) {
        sockets[0] = wsstream(new WebSocket('ws://localhost:3002/'));
        sockets[0].on('open', function() {
            debug('client socket open, piping socket into client 0 stream');         
            sockets[0].pipe(clients[0].createStream()).pipe(sockets[0]);
            done();
        });
    });

    it('should be able to send a message from client 0 to the room', function(done) {
        room.on('update', function(update) {
            console.log('room received: ', update);
        });

        clients[0].send('hi');
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
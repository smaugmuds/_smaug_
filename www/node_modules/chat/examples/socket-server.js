var chat = require('..'),
    room = chat.room(),
    net = require('net'),
    server = net.createServer(function(conn) {
        // establish two-way comms with the room on behalf of the remote client
        conn.pipe(room.connect()).pipe(conn);
    });

// start the server listening
server.listen(7080);
# node-chat load tests / benchmarks

The following files can be found in this directory:

## load.js

This file does a simple load test, where:

1. A single room is created.
2. A new client instance is connected to that room, the client says hello.
3. Once the client receives verification that the room has received the message, goto 2 and repeat.

## load-multiroom.js

As per the `load.js` test but two rooms are created and piped to one another.

## load-rooms.js

1. A central room instance is created.
2. A new room is created and piped to the central room.
3. A client that is connected to the new room.
3. As per `load.js` the client sends a message and awaits verification that it has been sent.
4. Once message send verification is confirmed, go back to step 2 and repeat.
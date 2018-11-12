# Chat

Pure Streaming Chat for Node.  This is a simple library which implements the core concepts of chat (rooms and connections) without getting tying chat to any particular transport.

## Getting Started

The first thing you are going to want to do is create a room:

```js
var chat = require('chat'),
    room = chat.room();
```

Once you have a room, you can then start connecting clients to the room:

```js
var client = chat.client(room);
```

At this point, we should probably start listening for messages in the room:

```js
room.on('message', function(msg) {
    if (msg.data) {
        console.log(msg.data);
    }
});
```

The code above looks specifically for just the data messages that are captured by the room.  In addition to the data messages, control messages are also sent which can be captured by examining the `type` property of the message.

At this point, it's probably worth attempting to communicate with the room.  This is done using the `write` method of the client:

```js
client.write('hello');
```

Strange, we didn't capture any output.  This is because as far as the room is concerned, an unknown client is attempting to send messages and this isn't permitted.  While the chat package doesn't perform any authentication it does assume that a package using chat (such as [iceman](https://github.com/DamonOehlman/iceman)) will.  So at a base level, it requires identification:

```js
client.identify({ nick: 'Bill' });
```

Calling `identify` on the client stream kicks off a handshake process with the room, and thus we should wait for the client to become ready before writing any messages:

```js
client.once('ready', function(metadata) {
    client.write('hello'); 
});
```

For the full example, see [examples/hello.js](examples/hello.js).

## Transport Free

Obviously, this isn't the first chat implementation written in Node.  It is, however, one of the first to be written that isn't tied to a specific transport implementation (usually WebSockets).

An example of a pure socket level chat server and client can be found in the [examples](examples/) directory.
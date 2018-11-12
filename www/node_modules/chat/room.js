var debug = require('debug')('chat-room'),
    Doc = require('crdt/doc'),
    util = require('util'),
    uuid = require('uuid'),
    MuxDemux = require('mux-demux');

function Chatroom(opts) {
    if(!(this instanceof Chatroom)) return new Chatroom(opts);

    Doc.call(this, opts);

    // ensure we have opts
    opts = opts || {};

    // init
    this.store = {};

    // initialise the backlog size (default to 100 messages)
    this.backlogSize = typeof opts.backlogSize != 'undefined' ? opts.backlogSize : 100;

    // initialise the messages
    this.messages = this.createSeq('type', 'message');

    // initialise the room members
    this.connections = this.createSet('type', 'connection');

    // open the chat instance
    this.open();
}

util.inherits(Chatroom, Doc);
module.exports = Chatroom;

/**
## open()
*/
Chatroom.prototype.open = function() {

    var room = this;

    function messageAdd(row) {
        // split the row id to get the user and time details
        var parts = row.id.split('|'),
            ticks = parseInt(parts[0]),
            connection = room.connections.rows[parts[1]];

        room.emit('message', {
            data: row.state.data,
            time: ticks,
            cid:  parts[1]
        });
    }

    function messageRemove(row) {
        // split the row id to get the user and time details
        var parts = row.id.split('|'),
            ticks = parseInt(parts[0]);

        debug('captured message removal, triggering MSGREMOVE event');
        room.emit('message', {
            type: 'MSGREMOVE',
            time: ticks,
            cid:  parts[1]
        });
    }

    function connectionAdd(row) {
    }

    function connectionChanged(row, changed) {
        if (changed) {
            // if the user is changed, then process authentication 
            if (changed.user && (! row.state.authenticated)) {
                debug('detected user details changed, running authentication');

                // if we have authentication listeners, then trigger with the user details
                if (room.listeners('authenticate').length > 0) {
                    room.emit('authenticate', row.state.user, row);
                }
                // otherwise, update the user and supply the authenticated flag
                else {
                    row.set('authenticated', true);                
                }
            }
            
            if (changed.authenticated === true) {
                room.emit('message', {
                    type: 'USERJOIN',
                    time: new Date(),

                    cid:  row.id,
                    user: row.state.user
                });        
            }
        }
    }

    function connectionRemove(row) {
        room.emit('message', {
            type: 'USERLEAVE',
            time: new Date(),

            cid:  row.id
        });
    }

    // wire up event handler
    this.messages.on('add', messageAdd);
    this.messages.on('remove', messageRemove);
    this.connections.on('add', connectionAdd);
    this.connections.on('changes', connectionChanged);
    this.connections.on('remove', connectionRemove);

    // add a close function override
    this.close = function() {
        // remove the event handlers
        this.messages.removeListener('add', messageAdd);
        this.connections.removeListener('add', connectionAdd);
        this.connections.removeListener('changes', connectionChanged);
        this.connections.removeListener('remove', connectionRemove);

        // remove this override
        this.close = undefined;
    }
};

/**
## connect(details)

Connect to the the chat room with the uid and details specified.  This function 
returns a MuxDemux stream that can be used to push messages into the 
room.
*/
Chatroom.prototype.connect = function() {
    var id = uuid.v4(),
        mdm, 
        room = this, 
        connection;

    // add the connection
    debug('adding a new connection to the chat room, cid: ' + id);
    connection = this.add({ id: id, type: 'connection' });

    // create the muxdemux instance
    mdm = MuxDemux();
    mdm.on('connection', function(stream) {
        debug('received stream for room connection: ' + id);

        function handleMessage(msg) {
            // only write messages out if the user is authenticated
            if (connection.state.authenticated) {
                // rewrite userjoin and userleave events as join and leave
                // for the current connection
                if (msg.cid === id && msg.type && msg.type.slice(0, 4) === 'USER') {
                    // create a new version of the message for the user
                    msg = {
                        type: msg.type.slice(4),
                        data: msg.data,
                        time: msg.time,

                        cid:  msg.cid,
                        user: msg.user,

                        // add the room meta data
                        meta: room.metadata
                    };
                }

                debug('--> stream data (cid: ' + id + '):', msg);
                stream.write(msg);
            }
        }

        // increment the count of streams for this connection
        connection.streams = (connection.streams || 0) + 1;

        // handle close events
        stream.on('close', function() {
            debug('stream closed - decoupling message handler');
            room.removeListener('message', handleMessage);

            // decrement the streams count
            if (connection.streams) {
                connection.streams -= 1;
            }

            // if we have no streams connected, then remove the connection
            if (! connection.streams) {
                room.connections.remove(id);
            }
        });

        if (stream.readable) {
            stream.on('data', function(data) {
                debug('<-- stream data (cid: ' + id + '): ', data);

                if (typeof data == 'string' || (data instanceof String) || (data instanceof Buffer)) {
                    room.processMessage(connection, data);
                }
                else if (typeof data == 'object') {
                    switch (data.type) {
                        // process known types
                        case 'ident':
                            room.processIdent(connection, data.user, data.permissions);
                            break;

                        // handle unknown types, fallback to message passing
                        default:
                            room.processMessage(connection, data.data || data.message || data.text || data);
                            break;
                    }
                }
            });
        }

        if (stream.writable) {
            // if the user is authenticated, then send messages
            room.on('message', handleMessage);
        }
    });

    // add the uid to the muxdemux object
    mdm.id = id;

    return mdm;
};

/**
## processIdent(connection, data)
*/
Chatroom.prototype.processIdent = function(connection, user, permissions) {
    debug('processing ident for connection: ' + connection.id, user, permissions);

    // set the user details for the connection
    connection.set('user', user);

    // if we have permissions set those details
    if (typeof permissions != 'undefined') {
        connection.set('permissions', permissions);
    }
};

/**
## processMessage(connection, text)
*/
Chatroom.prototype.processMessage = function(connection, text) {
    var id;

    if ((! connection.state.authenticated) || (connection.state.permissions === 'readonly')) return;

    this.add({
        id: new Date().getTime() + '|' + connection.id, 
        type: 'message', 
        data: text 
    });
};

/**
## metadata property
*/

Object.defineProperty(Chatroom.prototype, 'metadata', {
    get: function() {
        var validConnections = this.connections._array.filter(function(conn) {
                return conn.state.authenticated;
            }),
            msgCount = this.messages._array.length;

        return {
            // provide message backlog
            backlog: this.messages._array.slice(Math.max(0, msgCount - this.backlogSize), this.backlogSize),

            // provide connection information
            connections: validConnections.map(function(conn) {
                return {
                    id: conn.state.id,
                    user: conn.state.user
                };
            })
        };
    }
});
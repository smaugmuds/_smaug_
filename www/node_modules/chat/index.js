var debug = require('debug')('chat-client'),
    Chatroom = require('./room'),
    MuxDemux = require('mux-demux');

exports.client = require('./client');
exports.room = function(opts) {
    return new Chatroom(opts);
}
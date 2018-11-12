/***/
var http = require('http'), https = require('https');
var events = require('events'), util = require('util');

var Socket = require('./socket');

module.exports = WebSocketServer;

var evts = ['request', 'connection', 'close', 'checkContinue', 'upgrade',
    'clientError'];
var fncs = ['listen', 'close'];

function WebSocketServer(options) {
  var self = this;
  events.EventEmitter.call(self);

  var server, opts = 'object' === typeof options ? options: {};

  if('server' in opts)
    server = opts['server'], delete opts['server'];

  else {
    var prtcl = 'key' in opts && 'cert' in opts ? https: http;
    server = prtcl.createServer.apply(prtcl, arguments);
  }

  evts.forEach(function(evt) {
    _bubble(evt, server, self);
  });

  fncs.forEach(function(fnc) {
    _wrap(fnc, server, self);
  });

  var connections = {};

  self.on('upgrade', function(req, socket, upgradeHead) {
    var conn = new Socket(req, socket, upgradeHead, opts);
    var _socket = conn._socket = socket.soket || socket;
    var site = _socket.remoteAddress;
    var port = _socket.remotePort;

    conn._req = req;
    
    conn.on('open', function() {
      var internal = connections[site] = connections[site] || [];
      // conn['_id'] = port;
      internal.push(conn);
      self.emit('connect', conn);
      self.emit('handShake', conn);

    });

    conn.on('close', function() {
      var internal = connections[site] || [];
      var index = internal.indexOf(conn);
      0 <= index && internal.splice(index, 1);

      self.emit('release');
    });

  });
}
util.inherits(WebSocketServer, events.EventEmitter);

WebSocketServer.prototype.broadcast = function() {
};

function _bubble(evt, from, to) {
  from.on(evt, function() {
    var args = Array.prototype.slice.call(arguments);
    args.unshift(evt);
    events.EventEmitter.prototype.emit.apply(to, args);
  });
}

function _wrap(fnc, from, to) {
  to[fnc] = function() {
    return from[fnc].apply(from, arguments);
  };
}

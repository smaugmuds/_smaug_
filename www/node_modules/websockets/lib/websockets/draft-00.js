/***/
var util = require('util'), events = require('events'), crypto = require('crypto');
var URL = require('url');

var HANDSHAKE_RESPONSE = 'HTTP/1.1 101 WebSocket Protocol Handshake\r\nUpgrade: WebSocket\r\nConnection: Upgrade\r\n';

module.exports = exports = Draft00;

function Draft00(arg0, arg1) {
  var self = this;
  events.EventEmitter.call(self);

  var isClient = 'string' === typeof arg0;

  var Core = isClient ? Client: Server;
  self.core = new Core(self);

  var opts = self._options = (isClient ? arguments[1]: arguments[3]) || {};
  if('string' === typeof opts)
    opts = self._options = {
      protocols: [opts]
    };
  else if('array' === typeof opts)
    opts = self._options = {
      protocols: opts
    };
}
util.inherits(Draft00, events.EventEmitter);

function _option(key) {
  return this._options[key];
}

function Server(parent) {
  this.parent = parent;
}
function Client(parent) {
  this.parent = parent;
}

// ---------------------------------------------------
// close
// ---------------------------------------------------
Server.prototype.close = function(reason) {
  close.call(this, reason);
};
Draft00.prototype.close = function(reason) {
  this.core.close(reason);
};
function close(reason, maskbit) {
  var socket = this.parent.socket;
  socket.writable && socket.end();
  socket.destroy();
}

//---------------------------------------------------
//ping
//---------------------------------------------------
Server.prototype.ping = function(mess) {
  this.parent.emit('pong', mess);
};
Client.prototype.ping = function() {
};
Draft00.prototype.ping = function(mess) {
  this.core.ping(mess);
};

// ---------------------------------------------------
// write
// ---------------------------------------------------
Server.prototype.write = function(massage) {
  write.call(this, massage);
};
Draft00.prototype.write = function(massage) {
  this.core.write(massage);
};
function write(message, maskbit) {
  var socket = this.parent.socket;

  var length = Buffer.byteLength(message);
  var mess = new Buffer(length + 2);

  mess[0] = 0;
  mess.write(message, 1);
  mess[length + 1] = 0xFF;

  socket.write(mess);

}

// ---------------------------------------------------
// parse
// ---------------------------------------------------
Server.prototype.parse = function(data) {
  parse.call(this, data);
};
Draft00.prototype.parse = function(data) {
  this.core.parse(data);
};
function parse(data) {
  var self = this, parent = self.parent;
  self.buffer = self.buffer || [];

  var i = 0, l = data.length;
  
  try {
    frame();
  } catch(err) {
    parent.close();
  }

  function frame() {
    var type = self.type == null ? self.type = data[i++]: self.type;
    var sb = type >>> 7;
    if(sb) {
      if(0xFF !== type)
        throw new Error();// abort
      var length = 0, b;
      for(; i < l && b !== 0; i++)
        b = _length(data[i]);

    } else {
      if(0 !== type)
        throw new Error();// abort
      for(; i < l;) {
        var b = data[i++];
        0xFF === b ? _flush(): _data(b);
      }
    }

    function _data(d) {
      self.buffer.push(d);
    }
    function _length(d) {
      // TODO
    }
    function _flush() {
      var mess = self.buffer;
      parent.emit('message', Buffer(mess).toString());
      self.buffer = [], self.type = null;
      if(i < l)
        frame();
    }
  }
}

// ---------------------------------------------------
// hand shake
// ---------------------------------------------------
Draft00.prototype.handShake = function() {
  var core = this.core;
  core.handShake.apply(core, arguments);
};
Server.prototype.handShake = function(req, socket, upgradeHead) {
  var parent = this.parent, head = req['headers'];
  parent.socket = socket;

  if('WebSocket' !== head['upgrade'] || 'Upgrade' !== head['connection'])
    return socket.write(ERR_400);

  var host = head['host'], _origin = head['origin'];
  var _key1 = head['sec-websocket-key1'], _key2 = head['sec-websocket-key2'];
  var _key3 = upgradeHead;
  var _secure = !!req.socket.encrypted;
  var protocol = _secure ? 'wss://': 'ws://';

  var url = URL.parse(protocol + host);
  var _host = url.hostname, _port = url.port, _resource = url.pathname;

  var _location = protocol + _host;
  if(_port && +_port !== (_secure ? 443: 80))
    _location += ':' + _port;
  _location += req.url;

  function makeKey(key) {
    var i, len, num = '', cnt = 0;
    for(i = 0, len = key.length; i < len; i++)
      if(key[i].match(/\d/))
        num += key[i];
      else if('\u0020' === key[i])
        cnt++;
    var part = ~~(+num / cnt);
    return Buffer([part >>> 24 & 0xFF, part >>> 16 & 0xFF, part >>> 8 & 0xFF,
        part & 0xFF]);
  }

  var open = true, close = function() {
    if(open)
      open = false, parent.emit('close');
  };

  socket.on('close', close);

  socket.on('end', close);

  socket.on('data', function(data) {
    parent.parse(data);
  });

  var _part1 = makeKey(_key1), _part2 = makeKey(_key2);

  var hash = crypto.createHash('md5');
  hash.update(_part1);
  hash.update(_part2);
  hash.update(_key3);

  var head = HANDSHAKE_RESPONSE;
  head += 'Sec-WebSocket-Origin: ' + _origin + '\r\n';
  head += 'Sec-WebSocket-Location: ' + _location + '\r\n\r\n';
  socket.write(head + hash.digest(), 'binary');
  parent.emit('connect', socket);

  function _isAllowed() {
    var validate = parent._options['validateOrigin'];
    return 'function' === typeof validate ? validate(head): true;
  }
};

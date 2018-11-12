/***/
var util = require('util'), events = require('events'), crypto = require('crypto');
var URL = require('url'), http = require('http'), https = require('https');

var GUID = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11';
var HANDSHAKE_RESPONSE = 'HTTP/1.1 101 Switching Protocols\r\nSec-WebSocket-Protocol: *\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ';
var ERR_400 = 'HTTP/1.1 400 Bad Request\r\n\r\n400 Bad Request';
var ERR_403 = 'HTTP/1.1 403 Forbidden\r\n\r\n403 Forbidden';
var ERR_426 = 'HTTP/1.1 426 Upgrade Required\r\n\r\n426 Upgrade Required';

module.exports = exports = Draft10;

function Draft10(arg0, arg1) {
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

  var bufSize = opts['bufferSize'];
  if(!bufSize || bufSize <= 0)
    opts['bufferSize'] = BUFFER_SIZE;

  self.on('_data', function(data, fin, opcode) {
    if(0 !== opcode)
      self.opcode = opcode, self.buffer = [];
//    Array.prototype.push.apply(self.buffer, data);
    self.buffer = self.buffer.concat(data);
    self.emit('data', new Buffer(data), fin, self.opcode);
    if(fin) {
      var buf = new Buffer(self.buffer);
      var message = 1 === self.opcode ? buf.toString(): buf;
      self.emit('message', message);
    }
  });
}
util.inherits(Draft10, events.EventEmitter);

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
  close.call(this, reason, 0);
};
Client.prototype.close = function(reason) {
  close.call(this, reason, 1);
};
Draft10.prototype.close = function(reason) {
  this.core.close(reason);
};
function close(reason, maskbit) {
  var self = this, parent = self.parent;
  var message = ERR_CODE[reason] || (reason + '');
  _ctrl.call(self, OPCODE['close'], maskbit, message);

  parent.close = function() {
    var socket = parent.socket;
    socket.writable && socket.end();
    socket.destroy();
  };
}

// ---------------------------------------------------
// Control Frames
// ---------------------------------------------------
function _ctrl(opcode, maskbit, data) {
  var parent = this.parent, buf = new Buffer(data);
  if(125 < buf.length)
    buf = buf.slice(0, 124);

  var fin = 1;
  var ext = [0x00, 0x00, 0x00];

  parent.socket.write(frame(fin, ext, opcode, maskbit, buf));

}

// ---------------------------------------------------
// write
// ---------------------------------------------------
Server.prototype.write = function(massage) {
  write.call(this, massage, 0);
};
Client.prototype.write = function(massage) {
  write.call(this, massage, 1);
};
Draft10.prototype.write = function(massage) {
  this.core.write(massage);
};
function write(message, maskbit) {
  var parent = this.parent, socket = parent.socket;

  var buf = new Buffer(message), len = buf.length;

  var i, data = [], size = parent._options['bufferSize'];
  for(i = 0; i < len; i += size)
    data.push(buf.slice(i, Math.min(buf.length, i + size)));

  var fin = 0;
  var ext = [0, 0, 0];
  var opcode = OPCODE[typeof message] || OPCODE['binary'];
  var mask = maskbit;

  i = 0, len = data.length;

  for(; i < len - 1; i++, opcode = 0)
    socket.write(frame(fin, ext, opcode, mask, data[i]));

  fin = 1;
  socket.write(frame(fin, ext, opcode, mask, data[i]));

}

// ---------------------------------------------------
// pong
// ---------------------------------------------------
Draft10.prototype.pong = Draft10.prototype.write;

// ---------------------------------------------------
// parse
// ---------------------------------------------------
Server.prototype.parse = function(data) {
  parse.call(this, data, 1);
};
Client.prototype.parse = function(data) {
  parse.call(this, data, 0);
};
Draft10.prototype.parse = function(data) {
  this.core.parse(data);
};
function parse(data, maskbit) {
  var self = this, parent = self.parent, i = 0;

  var OP_FUNC = {
    _def: function(payload, fin, opcode) {
      parent.emit('_data', payload, fin, opcode);
    },
    0x00: function(payload, fin) {
      OP_FUNC['_def'](payload, fin, 0);
    },
    0x01: function string(payload, fin) {
      OP_FUNC['_def'](payload, fin, 1);
    },
    0x02: function binary(payload, fin) {
      OP_FUNC['_def'](payload, fin, 2);
    },
    0x08: function close() {
      parent.emit('_closing', 1000);
    },
    0x09: function ping(payload) {
      self.pong(payload);
    },
    0x0A: function pong(payload) {
      // TODO
    }
  };

  if('undefined' === typeof self.buffer)
    _parseHead();

  _parsePayload();

  function _parseHead() {

    var byte = data[i++];
    var fin = byte >>> 7;
    var ope = (byte & 0x0F);

    byte = data[i++];
    var mask = byte >>> 7;
    if(maskbit !== mask)
      return parent.emit('error', 1002);
    var len = byte & 0x7F;

    if(127 === len)
      len = ((data[i++] << 24) + (data[i++] << 16) + (data[i++] << 8) + data[i++]) * Math
          .pow(2, 32) + (data[i++] << 24) + (data[i++] << 16) + (data[i++] << 8) + data[i++];
    else if(126 === len)
      len = (data[i++] << 8) + data[i++];

    var maskkey = maskbit ? [data[i++], data[i++], data[i++], data[i++]]: null;

    self.buffer = {
      fin: fin,
      ope: ope,
      length: len,
      mask: maskkey,
      payload: []
    };
  };

  function _parsePayload() {
    var buf = self.buffer, len = data.length;
    var payload = buf.payload, maskkey = buf.mask, ope = buf.ope, fin = buf.fin;
    var j = payload.length % 4;
    for(; buf.length && i < len; j++, buf.length--)
      payload.push(maskkey ? data[i++] ^ maskkey[j % 4]: data[i++]);

    if('function' !== typeof OP_FUNC[ope])
      return parent.emit('error', 1002);

    if(0 === buf.length) {
      OP_FUNC[ope](payload, !!fin);
      delete self.buffer;
    }

    // TODO test
    if(i < len)
      parse.call(self, data.slice(i), maskbit);

  }
}

// ---------------------------------------------------
// hand shake
// ---------------------------------------------------
Draft10.prototype.handShake = function() {
  var core = this.core;
  core.handShake.apply(core, arguments);
};
Server.prototype.handShake = function(req, socket, upgradeHead) {
  var parent = this.parent, head = req['headers'];
  parent.socket = socket;

  var i, keys = ['host', 'sec-websocket-key'];
  for(i = keys.length; i--;)
    if(!(keys[i] in head))
      return socket.write(ERR_400);
  if('websocket' !== head['upgrade'])
    return socket.write(ERR_400);
  if('8' !== head['sec-websocket-version'])
    return socket.write(ERR_426);
  if(true !== _isAllowed())
    return socket.write(ERR_403);

  // TODO
  head['sec-websocket-protocol'], head['cookie'];

  var open = true, close = function() {
    if(open)
      open = false, parent.emit('close');
  };

  var newkey = _hashAndEncode(head['sec-websocket-key']);
  socket.write(HANDSHAKE_RESPONSE);
  socket.write(newkey + '\r\n\r\n');
  process.nextTick(function() {
    socket.on('close', close);
    socket.on('end', close);
    socket.on('data', function(data) {
      parent.parse(data);
    });
    parent.emit('connect', socket);
  });

  function _isAllowed() {
    var validate = parent._options['validateOrigin'];
    return 'function' === typeof validate ? validate(head): true;
  }
};
Client.prototype.handShake = function(uri) {
  var parent = this.parent, url = URL.parse(uri);
  parent.URL = uri;

  var secure = parent.secure = 'wss:' === url['protocol'];

  var URI = url['protocol'];
  URI += '//';
  URI += url['host'];
  URI += url['pathname'] || '/';
  URI += (url['search'] || '').replace(/#/, '%23');

  var key = _createKey();

  var opts = {
    port: url['port'] || (secure ? 443: 80),
    host: url['hostname'],
    headers: {
      Connection: 'Upgrade',
      Upgrade: 'websocket',
      'Sec-Websocket-Key': key,
      'Sec-Websocket-Origin': 'node-websockets',
      'Sec-Websocket-Version': 8
    }
  };

  if(secure) {
    opts['key'] = parent._options['key'];
    opts['cert'] = parent._options['cert'];
    opts['ca'] = parent._options['ca'];
  }

  var prtcl = secure ? https: http, agent;
  if(prtcl.getAgent) {
    agent = prtcl.getAgent(opts['host'], opts['port']);
    opts['agent'] = agent;
  }
  var req = prtcl.request(opts);
  req.end();
  agent = agent || req;

  var newkey = _hashAndEncode(key);

  agent.on('upgrade', function(res, socket, upgradeHead) {

    socket.on('close', function(had_error) {
      parent.emit('close', had_error);
    });

    socket.on('data', function(data) {
      parent.parse(data);
    });

    // TODO
    var status = res['statusCode'];
    if(101 !== status)
      return parent.emit('_closing', status);

    var head = res['headers'];
    if('websocket' !== head['upgrade'] || 'Upgrade' !== head['connection'])
      return parent.emit('error', new Error('Invalid Response Header'));

    if(head['sec-websocket-accept'] !== newkey)
      return parent.emit('error', new Error('Invalid Server Response'));

    parent.socket = socket;

    parent.emit('connect', socket);
  });
  req.on('error', function(err) {
    parent.emit('error', err);
  });

  function _createKey() {
    var i, arr = [];
    for(i = 16; i--;)
      arr.push(~~(Math.random() * 256));
    return (new Buffer(arr)).toString('base64');
  }
};

var OPCODE = {
  string: 0x01,
  binary: 0x02,
  close: 0x08,
  ping: 0x09,
  pong: 0x0A
};
var ERR_CODE = {
  1000: 'Normal Closure',
  1001: 'Going Away',
  1002: 'Protocol error',
  1003: 'Unsupported Data',
  1004: 'Frame Too Large',
  1005: 'No Status Rcvd', // MUST NOT be set in Close control
  1006: 'Abnormal Closure', // MUST NOT be set in Close control
  1007: 'Invalid UTF-8'
};

var BUFFER_SIZE = 2000000;

function _hashAndEncode(key) {
  var newkey = (key + GUID).trim();
  var shasum = crypto.createHash('sha1');
  shasum.update(newkey);
  return shasum.digest('base64');
}

function frame(fin, ext, opcode, mask, payload) {
  var arr = [];
  var push = Array.prototype.push;
  push.apply(arr, _1st(fin, ext, opcode));
  push.apply(arr, _length(mask, payload));
  arr = arr.concat(_payload(mask, payload));

  return new Buffer(arr);
}

function _1st(fin, ext, opcode) {
  return [(fin << 7) | (ext[0] << 6) | (ext[1] << 5) | (ext[2] << 4) | opcode];
}

function _length(mask, payload) {
  var length = payload.length, len = length, extlen = [];
  if(length <= 125)
    ;
  else if(length <= 0xFFFF) {
    len = 126;
    extlen.push(length >>> 8);
    extlen.push(length & 0xFF);
  } else {
    len = 127;
    // var zerofill =
    // '0000000000000000000000000000000000000000000000000000000000000000';
    var zerofill = '000000000000000000000000000000000000000000000000';
    var bin = (zerofill + length.toString(2)).slice(-64);
    extlen.push(parseInt(bin.substr(0, 8), 2));
    extlen.push(parseInt(bin.substr(8, 8), 2));
    extlen.push(parseInt(bin.substr(16, 8), 2));
    extlen.push(parseInt(bin.substr(24, 8), 2));
    extlen.push(parseInt(bin.substr(32, 8), 2));
    extlen.push(parseInt(bin.substr(40, 8), 2));
    extlen.push(parseInt(bin.substr(48, 8), 2));
    extlen.push(parseInt(bin.substr(56, 8), 2));
  }
  extlen.unshift((mask << 7) | len);
  return extlen;
}

function _payload(maskbit, payload) {
  var i, mask, ret, length;
  if(maskbit) {
    mask = makeMask(), ret = mask.slice(-4), length = payload.length;
    for(i = 0; i < length; i++)
      ret.push(payload[i] ^ mask[i % 4]);
  } else
    ret = Array.prototype.slice.call(payload);

  return ret;

  function makeMask() {
    var i, ret = [];
    for(i = 4; i--;)
      ret.push(~~(Math.random() * 256));
    return ret;
  }
}

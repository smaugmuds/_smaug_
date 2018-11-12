/***/
var Server = require('./server');
var Socket = require('./socket');

exports.Server = Server;
exports.WebSocket = Socket;

exports.createServer = (function() {
  function S(args) {
    return Server.apply(this, args);
  }
  S.prototype = Server.prototype;
  
  return function() {
    var args = arguments;
    return new S(args);
  };
})();

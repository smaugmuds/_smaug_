# redis-protocol-stream

Streams of the [redis protocol](http://redis.io/topics/protocol)

<img src=https://secure.travis-ci.org/dominictarr/redis-protocol-stream.png?branch=master>

The Redis Protocol is suitable for streams of arrays of binary data, 
while also being moderately human readable. 

``` js
var redis = require('redis-protocol-stream')

//convert arrays to the redis protocol.

var rs = redis.stringify()
  .pipe(process.stdout, {end: false})

rs.write(['hello', 3, 'whatever'])

var ps = redis.parse({buffers: false})
  .on('data', console.log)

ps.write('*3\r\n$5\r\nhello\r\n$5\r\nmello\r\n$5\r\nyello\r\n')

```
Also, convert a stream of arrays of strings or buffers to REDIS.

``` js
var redis = require('redis-protocol-stream')
var through = require('through')

var rs = redis.serialize(through(function (ary) {
  console.log(ary)
  this.emit('data', ary)
}))

```
This is useful when you have a duplex stream that you want in the redis protocol.

## License

MIT

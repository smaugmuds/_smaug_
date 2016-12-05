require('tape')('serialize', function (t) {

  var REDIS = require('..')

  var through = require('through')
  var data = ''
  var input = '*3\r\n$5\r\nhello\r\n$5\r\nmello\r\n$5\r\nyello\r\n'

  var rs = REDIS.serialize(through(function (data) {
      var out = data.map(function (e) { return e.toString().toUpperCase()})
      t.deepEqual(out, ['HELLO', 'MELLO', 'YELLO'])
      this.queue(out)
    }), {buffers: false})
    .on('data', function (_data) {
      data += _data
    })
    .on('end', function () {
      t.equal(data, input.toUpperCase())
      t.end()
    })

  rs.write(input)
  rs.end()

})

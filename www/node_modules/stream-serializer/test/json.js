
var through   = require('through')
var serialize = require('..')
var assert    = require('assert')

var expected = [1, 2, 3], actual = []
var ended = false

var ss = serialize()(through(function (data) {
  var n = expect = expected.shift()
  assert.equal(data, n)
  console.log(data)
  this.emit('data', data)
}, function () {
  ended = true
}))


ss.on('data', function (d) {
  console.log('>>', d, '<<')
  actual.push(d)
})
ss.write('1\n2\n3\n')
ss.end()

assert.equal(expected.length, 0)
assert.deepEqual(actual, ['1\n', '2\n', '3\n'])
assert.equal(ended, true)


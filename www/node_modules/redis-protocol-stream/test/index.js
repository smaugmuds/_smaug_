require('tape')('REDIS protocol stream', function (t) {

  var REDIS = require('..')

  //rs.pipe(process.stdout, {end: false})

  var input = [
    ['hello' , 'whatever'  , 'do something' ],
    ['hello2', 'whoever'   , 'did something'],
    ['hello3', 'whenever'  , 'do anything'  ],
    ['hello6', 'thatalways', '!'            ]
  ]
  //STRINGS

  var _input = input.slice(), strings = [], buffers = []
  var rs = REDIS.stringify()

  //strings if you pass buffers: false
  rs.pipe(REDIS.parse({buffers: false}))
    .on('data', function (ary) {
      strings.push(ary)
    })
    .on('end', function () {
      t.deepEqual(input, strings)
      console.log(strings)
    })

  //DEFALT TO BUFFERS
  rs.pipe(REDIS.parse())
    .on('data', function (ary) {
      buffers.push(
        ary.map(function (buf) {
          t.equal(Buffer.isBuffer(buf), true)
          return buf.toString()
        })
      )
    })
    .on('end', function () {
      t.deepEqual(input, buffers)
      console.log(buffers)
      console.log('passed')
    })

  while(_input.length)
    rs.write(_input.shift())
  rs.end()

  t.end()
})

var test = require('tape')
var u = require('../')
var a = require('assertions')

  function assertBetween (_, lo, hi, depth, between) {
    between = between || u.between
    var b = between(lo, hi)

/*
    console.log(depth)
    console.log('<<<<', lo)
    console.log('====', b)
    console.log('>>>>', hi)
/**/

    a.greaterThan(b , lo)
    a.lessThan(b , hi)

    if(!depth) return
    if(~~(Math.random()*2)) 
      assertBetween(a, lo, b, depth - 1, between)
    else
      assertBetween(a, b, hi, depth - 1, between)
    
  }


test('between', function (t) {

  assertBetween(t, '!', '~', 200)

  t.end()
})
/*
  same as above but this time, append a random string to each.
  (I'm gonna use this to generate concurrently ordered strings
  that are unlikely to collide)
*/
test('between2', function (t) {

  assertBetween(t, '!', '~', 200, function (a, b) {
    return u.between (a, b) + u.randstr(5)
  })

  t.end()
})

test('between3', function (t) {

  assertBetween(t, u.lo, u.hi, 200, u.inject('$&[{}(=*)+]!#~`'))

  t.end()
})


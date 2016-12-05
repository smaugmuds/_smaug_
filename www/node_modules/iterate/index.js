
//
// adds all the fields from obj2 onto obj1
//

var each = exports.each = function (obj,iterator){
 var keys = Object.keys(obj)
 keys.forEach(function (key){
  iterator(obj[key],key,obj) 
 })
}

var RX = /sadf/.constructor
function rx (iterator ){
  return iterator instanceof RX ? function (str) { 
      var m = iterator.exec(str)
      return m && (m[1] ? m[1] : m[0]) 
    } : iterator
}

var times = exports.times = function () {
  var args = [].slice.call(arguments)
    , iterator = rx(args.pop())
    , m = args.pop()
    , i = args.shift()
    , j = args.shift()
    , diff, dir
    , a = []
    
    i = 'number' === typeof i ? i : 1
    diff = j ? j - i : 1
    dir = i < m
    if(m == i)
      throw new Error('steps cannot be the same: '+m+', '+i)
  for (; dir ? i <= m : m <= i; i += diff)
    a.push(iterator(i))
  return a
}

var map = exports.map = function (obj, iterator){
  iterator = rx(iterator)
  if(Array.isArray(obj))
    return obj.map(iterator)
  if('number' === typeof obj)
    return times.apply(null, [].slice.call(arguments))  
  //return if null ?  
  var keys = Object.keys(obj)
    , r = {}
  keys.forEach(function (key){
    r[key] = iterator(obj[key],key,obj) 
  })
  return r
}

var findReturn = exports.findReturn = function (obj, iterator) {
  iterator = rx(iterator)
  if(obj == null)
    return
  var keys = Object.keys(obj)
    , l = keys.length
  for (var i = 0; i < l; i ++) {
    var key = keys[i]
      , value = obj[key]
    var r = iterator(value, key)
    if(r) return r
  }
}

var find = exports.find = function (obj, iterator) { 
  iterator = rx(iterator)
  return findReturn (obj, function (v, k) {
    var r = iterator(v, k)
    if(r) return v
  })
}

var findKey = exports.findKey = function (obj, iterator) { 
  iterator = rx(iterator)
  return findReturn (obj, function (v, k) {
    var r = iterator(v, k)
    if(r) return k
  })
}

var filter = exports.filter = function (obj, iterator){
  iterator = rx (iterator)

  if(Array.isArray(obj))
    return obj.filter(iterator)
  
  var keys = Object.keys(obj)
    , r = {}
  keys.forEach(function (key){
    var v
    if(iterator(v = obj[key],key,obj))
      r[key] = v
  })
  return r 
}

var mapKeys = exports.mapKeys = function (ary, iterator){
  var r = {}
  iterator = rx(iterator)
  each(ary, function (v,k){
    r[v] = iterator(v,k)
  })
  return r
}


var mapToArray = exports.mapToArray = function (ary, iterator){
  var r = []
  iterator = rx(iterator)
  each(ary, function (v,k){
    r.push(iterator(v,k))
  })
  return r
}

var path = exports.path = function (object, path) {

  for (var i in path) {
    if(object == null) return undefined
    var key = path[i]
    object = object[key]
  }
  return object
}

/*
NOTE: naive implementation. 
`match` must not contain circular references.
*/

var setPath = exports.setPath = function (object, path, value) {

  for (var i in path) {
    var key = path[i]
    if(object[key] == null) object[key] = ( 
      i + 1 == path.length ? value : {}
    )
    object = object[key]
  }
}

var join = exports.join = function (A, B, it) {
  each(A, function (a, ak) {
    each(B, function (b, bk) {
      it(a, b, ak, bk)
    })
  })
}

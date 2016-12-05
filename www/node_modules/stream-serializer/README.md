# stream-serializer

wrap a stream of regular js objects into a stream of line seperated json.

## example

``` js
var json = require('stream-serializer').json

json(stream) //return a stream that can be piped through a text stream.
```

or

``` js
var ss require('stream-serializer')

//get your user to pass in a wrapper option
//which may be the typeof stream that is supported
//of a function that applys the wrapping.

ss(opts.wrapper)(stream) 

```

## License

MIT

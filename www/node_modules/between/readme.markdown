# between

generate a string that is between two strings.

## Example

there is an infinite number of numbers between 0 and 1.
0.5, 0.25, 0.00000000000000000000001, etc.

I wrote this module because I needed it for
[crdt](https://github.com/dominictarr/crdt).

I used it to mark the position of items in a sequence.

Say, A, B, C are inserted into positions 1, 2, 3.  
then, insert D inbetween A & B, into position 1.5. then move
C to position 1.25 and B to position 1.125. Now the list is A, B, C, D.

Seems like a funny way to track the position of elements right?

This method has a very important property: it is commutative.
concurrent inserts behave nicely, because inserting between two 
items does not change the positions of nodes later in the list!

## API

### between(a, b)

``` js

var b  = require('between')
var tween = b.between('a', 'b')

//tween is a string that will sort between 'a' and 'b'.


```

`a` and `b` can be any string that does not end in the lowest character.
this is like how there is only one number between 0 and 1 that ends in a "0".

### between.hi, between.lo

the top most string possible, and the lowest string possible. You must not be used
as positions for items, because it will not be possible to insert items after
or before them. These values represent the space infront of the first item, and the space between the last item.


# HashTable
An implementation of the own Hash Table with Separate Chaining

## Constructors:
0. The default constructor.
0. The constructor by begin-end iterators of `std::pair<key, value>` sequence.
0. The constructor by a `std::initializer_list` of `std::pair<key, value>`.
Сonstructors must support the ability to pass a `hasher` object of type Hash as the last argument. If it is not passed, the constructor by default is used.

## Iterators:
Forward iterators: `iterator` addresses `std::pair<const KeyType, ValueType>&` and `const_iterator` addresses `const std::pair<const KeyType, ValueType>&`

## Methods:
0. `size()`
0. `empty()`
0. `hash_function()` returns `hasher` by value
0. `insert(std::pair<key, value>)`. If such key already exists, method does nothing
0. `eraser(key)`. If such key doesn't exist, method does nothing
0. `begin()` for both iterator types
0. `end()` for both iterator types
0. `find()` for both iterator types
0. `operator []`
0. `at(key)`. If such key doesn't exist, throw `std::out_of_range`
0. `clear()`

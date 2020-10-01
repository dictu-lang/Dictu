---
layout: default
title: Collections
nav_order: 5
---

# Collections
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## Lists

Lists in Dictu are a type that allows you to store more than one value in a single data structure. Lists will automatically resize for you, and can hold any data type within them.

```js
var myList = [1, 2, "hi", true, false, nil];
```

### Indexing

```js
var myList = [1, 2, 3];
// Lists are 0 indexed.
print(myList[0]); // 1
```

Updating a value within a list uses the same syntax, except you supply a value via assignment.

```js
var myList = [1, 5, 3];
myList[1] = 2; // [1, 2, 3]
```

### Slicing

Slicing is technique used when we wish to return part of a list. It has a syntax of <list>[start:end] where start and end are optional
as long as one is provided.

E.g `[1, 2, 3, 4, 5][1:]` or `[1, 2, 3, 4, 5][:5]` are both valid. The start index is inclusive but the end index is exclusive.

```py
[1, 2, 3, 4, 5][1:]; // [2, 3, 4, 5]
[1, 2, 3, 4, 5][:4]; // [1, 2, 3, 4]
[1, 2, 3, 4, 5][1:4]; // [2, 3, 4]
[1, 2, 3, 4, 5][2:4]; // [3, 4]
```
### Adding to lists
#### list.push(value)

To add append a new value to a list, use the `.push()` method.

```js
var myList = [];
myList.push(10); // [10]
myList.push(11); // [10, 11]
```

#### list.insert(value, number)

To insert a value into a list at a given index without replacing the value use .insert().

```js
var x = [10, 12];

// Can insert to a list. This will not replace the value.
myList.insert(11, 1); // [10, 11, 12]
```

#### + Operator

Using the + operator on lists will join two lists and return a new list.
It must be used on two values which are both lists.

```js
var x = [10];

x + [11, 12];
print(x); // [10, 11, 12]
```

#### list.extend(list)

Similar to the + operator however this mutates the list the method is called on rather than returning a new list.

**Note:** values are not copied to the new list, they are just referenced. This means if the value is mutable
it will mutate in the extended list as well.

```js
var x = [];
x.extend([1, 2, 3]);
print(x); // [1, 2, 3]
x.extend([1, 2, 3]);
print(x); // [1, 2, 3, 1, 2, 3]
```

### list.toString()

Converts a list to a string.

```js
["1", 11].toString();        // ['1', 11]
["1", [11, "1"]].toString(); // ['1', [11, '1']]
```
### list.len()

Returns the length of the given list.

```js
[1, 2, 3].len(); // 3
```

### list.toBool()

Converts a list to a boolean. A list is a "truthy" value when it has a length greater than 0.

```js
[].toBool(); // false
[1].toBool(); // true
[[]].toBool(); // true
```

### list.contains(value)

To check if a value contains within a list we use `.contains()`

```js
var myList = [1, 2, 3];
myList.contains(2); // true
myList.contains(10); // false
```

### list.join(string: delimiter -> optional)

To convert a list of elements to a string use `.join()` to concatenate elements together by a given delimiter.
If a delimiter is not supplied `", "` is the default.

```js
var myList = [1, 2, 3];
print(myList.join()); // "1, 2, 3"
print(myList.join("")); // "123"
print(myList.join("-")); // "1-2-3"
```

### list.remove(value)

To remove a value from a list use `.remove()`. If the value does not exist within
the list a runtime error occurs. Use together with [list.contains()](#listcontainsvalue).

Note: If a list contains multiple values which are the same, a call to `.remove()` will only
remove the first occurrence, not all.

```js
var myList = [1, 2, 3];
myList.remove(3);
print(myList); // [1, 2]
myList.remove(1);
print(myList); // [2]
```

### list.pop(number: index -> optional)

To remove a value from a list, with an optional index, use `.pop()`

```js
var myList = [1, 2, 3];
// If no index is given, pops from the end
var someNumber = myList.pop(); // 3
print(myList); // [1, 2]
myList.pop(0); // 1
print(myList); // [2]
```

### Copying lists
#### list.copy()

When you are working with a mutable datatype taking a reference of a list when creating a new variable will modify the original list.
```js
var list1 = [1, 2];
var list2 = list1;
list2[0] = 10;
print(list1); // [10, 2]
```
To get around this we can make copies of the list. Dictu offers the ability to both shallow and deep copy a list.
```js
var list1 = [1, 2];
var list2 = list1.copy(); // shallow copy
list2[0] = 10;
print(list1); // [1, 2]
print(list2); // [10, 2]
```

#### list.deepCopy()
To get around this, we can deepCopy the list.
```js
var list1 = [[1, 2]];
var list2 = list1.deepCopy();
list2[0][0] = 10;
print(list1); // [[1, 2]]
print(list2); // [[10, 2]]
```

### Sorting Lists
#### list.sort()

To sort numeric lists (that is lists that contain only numbers) you can use the method
sort.

```js
var list1 = [1, -1, 4, 2, 10, 5, 3];

print(list1); // [1, -1, 4, 2, 10, 5, 3]
list1.sort();

print(list1); // [-1, 1, 2, 3, 4, 5, 10]
```

## Dictionaries

Dictionaries are a key:value pair data type. Dictu requires that the dictionary key be an immutable type (nil, boolean, number, string), however the value can be any type.

```js
var myDict = {"key": 1, "key1": true};
```

### Indexing

Accessing dictionary items is the same syntax as lists, except instead of an index, it expects an immutable type (nil, boolean, number, string) for it's key.
If you try to access a key that does not exist, `nil` is returned. If you expect a key may not exist `.get()` can be used to return a default value.

```js
var myDict = {"key": 1, "key1": true};
var someItem = myDict["key"]; // 1
var nilValue = myDict["unknown key"]; // nil
```

Updating a value within a dictionary uses the same syntax, except you supply a value via assignment.

```js
var myDict = {"key": 1, "key1": true};
var myDict["key"] = false; // {"key": false, "key1": true}
```

Adding a value to a dictionary is the same as updating a value, however if the key does not exist it is created.

```js
var myDict = {"key": 1, "key1": true};
var myDict["key2"] = nil; // {"key": false, "key1": true, "key3": nil}
```

### dict.get(string, value: default -> optional)

Returns the dictionary value at the given key, or returns the default value if the key does
not exist in the dictionary. If the key does not exist and no default is provided `nil` is returned.

```js
var myDict = {};

myDict.get("unknown key", "No key!"); // No key!
myDict.get("unknown key", 10); // 10
myDict.get("unknown key"); // nil
```

### dict.keys()

Returns a list of all of the dictionary keys.

```js
var myDict = {1: 2, "test": 3};

myDict.keys(); // [1, "test"]
```

### dict.toString()
Converts a dictionary to a string.

```js
{"1": 1, 1: "1"}.toString(); // '{"1": 1, 1: "1"}'
{"1": {1: "1", "1": 1}, 1: "1"}.toString(); // '{"1": {"1": 1, 1: "1"}, 1: "1"}'
```

### dict.toBool()

Converts a dictionary to a boolean. A dictionary is a "truthy" value when it has a length greater than 0.

```js
var x = {};

x.toBool(); // false
x["test"] = 1;
x.toBool(); // true
```

### dict.len()

Returns the length of the given dictionary.

```js
{1: "one", 2: "two", 3: "three"}.len(); // 3
```

### dict.exists(string)

To check if a key exists within a dictionary use `.exists()`

```js
var myDict = {"key": 1, "key1": true};
var keyExists = myDict.exists("key"); // true
var keyDoesNotExist = myDict.exists("unknown key"); // false
```

### dict.remove(string)

To remove a key from a dictionary use `.remove()`.

**Note**: If you try to remove a key that does not exist a runtime error is raised, use together with `.exists()`.

```js
var myDict = {"key": 1, "key1": true};
myDict.remove("key");
myDict; // {'key1': true}
myDict.remove("unknown key"); // [line 1] in script: Key 'unknown key' passed to remove() does not exist
```

### Copying dictionaries

This is the exact same scenario as lists, so refer to [copying Lists](#copying-lists) for information as to what is happening.

```js
var myDict = {"key": {"test": 10}};
var myDict1 = myDict.copy(); // Shallow copy
var myDict2 = myDict.deepCopy(); // Deep copy
```

## Sets

Sets are an unordered collection of unique hashable values. Set values must be of type string, number, boolean or nil.

```js
var mySet = set("test", 10);
print(mySet); // {10, "test"}
```

### set.toString()
Converts a given set to a string.

```js
var set_a = set();

set_a.add("one");
set_a.add("two");

var set_b = set();
set_b.add(1);
set_b.add(2);

set_a.toString(); // '{"two", "one"}');
set_b.toString(); // '{2, 1}'
```

### set.toBool()

Converts a set to a boolean. A set is a "truthy" value when it has a length greater than 0.

```js
var x = set();

x.toBool(); // false
x.add("test");
x.toBool(); // true
```

### set.len()

Returns the length of the given set.

```js
var mySet = set();
mySet.add("Dictu!");
mySet.len(); // 1
```

### set.add(value)

Adding to sets is just a case of passing a value to .add()

```js
var mySet = set();
mySet.add("Dictu!");
```

### set.contains(value)

To check if a set contains a value use `.contains()`

```js
var mySet = set();
mySet.add("Dictu!");
print(mySet.contains("Dictu!")); // true
print(mySet.contains("Other!")); // false
```

### set.remove(value)

To remove a value from a set use `.remove()`.

**Note**: If you try to remove a value that does not exist a runtime error is raised, use together with `.contains()`.

```js
var mySet = set();
mySet.add("Dictu!");
mySet.remove("Dictu!");
```

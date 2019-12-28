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

### Accessing list elements

```js
var myList = [1, 2, 3];
// Lists are 0 indexed.
print(myList[0]); // 1
```

### Updating a value
```js
var myList = [1, 5, 3];
myList[1] = 2; // [1, 2, 3]
```

### Adding to lists

```js
var myList = [];
myList.push(10); // [10]
myList.push(12); // [10, 12]
// Can insert to an index. This will not replace the value.
myList.insert(11, 1); // [10, 11, 12]
// You can also use the + operator to extend lists
myList + [13]; // [10, 11, 12, 13]
```

### Checking a value exists within a list

```js
var myList = [1, 2, 3];
myList.contains(2); // true
myList.contains(10); // false
```

### Removing from lists
```js
var myList = [1, 2, 3];
// If no index is given, pops from the end
var someNumber = myList.pop(); // 3
myList; // [1, 2]
myList.pop(0); // 1
myList; // [2]
```

### Copying lists
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
This works fine, however if you have a mutable datatype within the list on a shallow copy, you're back to square one.
```js
var list1 = [[1, 2];
var list2 = list1.copy();
list2[0][0] = 10;
print(list1); // [[10, 2]]
```
To get around this, we can deepCopy the list.
```js
var list1 = [[1, 2];
var list2 = list1.deepCopy();
list2[0][0] = 10;
print(list1); // [[1, 2]]
print(list2); // [[10, 2]]
```

## Dictionaries

Dictionaries are a key:value pair data type. Currently Dictu requires that the dictionary key be a string, however the value can be any type.

```js
var myDict = {"key": 1, "key1": true};
```

### Accessing dictionary values

Accessing dictionary items is the same syntax as lists, except instead of an index, it expects a string key.
If you try to access a key that does not exist, `nil` is returned. If you expect a key may not exist `.get()` can be used to return a default value.

```js
var myDict = {"key": 1, "key1": true};
var someItem = myDict["key"]; // 1
var nilValue = myDict["unknown key"]; // nil
// Using .get() for defaults
var defaultValue = myDict.get("unknown key", "No key!"); // No key!
```

### Updating a dictionary value

```js
var myDict = {"key": 1, "key1": true};
var myDict["key"] = false; // {"key": false, "key1": true}
```

### Adding to dictionaries

The exact same syntax as updating a value, just if a key does not exist, it is created.

```js
var myDict = {"key": 1, "key1": true};
var myDict["key2"] = nil; // {"key": false, "key1": true, "key3": nil}
```

### Checking if a key exists

```js
var myDict = {"key": 1, "key1": true};
var keyExists = myDict.exists("key"); // true
var keyDoesNotExist = myDict.exists("unknown key"); // false
```

### Removing from dictionaries

If you try to remove a key that does not exist a runtime error is raised.

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
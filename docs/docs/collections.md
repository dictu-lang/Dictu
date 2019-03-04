---
layout: default
title: Collections
nav_order: 4
---

# Variables
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
push(myList, 10); // [10]
push(myList, 12); // [10, 12]
// Can push to an index. This will not replace the value.
push(myList, 11, 1); // [10, 11, 12]
```

### Removing from lists
```js
var myList = [1, 2, 3];
// If no index is given, pops from the end
var someNumber = pop(myList); // 3
myList; // [1, 3]
```

## Dictionaries

Dictionaries are a key:value pair data type. Currently Dictu requires that the dictionary key be a string, however the value can be any type.

```js
var myDict = {"key": 1, "key1": true};
```

### Accessing dictionary values

Accessing dictionary items is the same syntax as lists, except instead of an index, it expects a string key.

```js
var myDict = {"key": 1, "key1": true};
var someItem = myDict["key"]; // 1
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

### Removing from dictionaries

TODO
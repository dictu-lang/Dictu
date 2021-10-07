---
layout: default
title: Dictionaries
nav_order: 2
parent: Collections
---

# Dictionaries
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## Dictionaries

Dictionaries are a key:value pair data type. Dictu requires that the dictionary key be an immutable type (nil, boolean, number, string), however the value can be any type.

```cs
var myDict = {"key": 1, "key1": true};
```

### Indexing

Accessing dictionary items is the same syntax as lists, except instead of an index, it expects an immutable type (nil, boolean, number, string) for it's key.
If you try to access a key that does not exist, a runtime error will be raised. If you expect a key may not exist `.get()` can be used to return a default value.

```cs
var myDict = {"key": 1, "key1": true};
var someItem = myDict["key"]; // 1
var nilValue = myDict["unknown key"]; // nil
```

Updating a value within a dictionary uses the same syntax, except you supply a value via assignment.

```cs
var myDict = {"key": 1, "key1": true};
var myDict["key"] = false; // {"key": false, "key1": true}
```

Adding a value to a dictionary is the same as updating a value, however if the key does not exist it is created.

```cs
var myDict = {"key": 1, "key1": true};
var myDict["key2"] = nil; // {"key": false, "key1": true, "key3": nil}
```

### dict.get(string, value: default -> optional)

Returns the dictionary value at the given key, or returns the default value if the key does
not exist in the dictionary. If the key does not exist and no default is provided `nil` is returned.

```cs
var myDict = {};

myDict.get("unknown key", "No key!"); // No key!
myDict.get("unknown key", 10); // 10
myDict.get("unknown key"); // nil
```

### dict.keys()

Returns a list of all of the dictionary keys.

```cs
var myDict = {1: 2, "test": 3};

myDict.keys(); // [1, "test"]
```

### dict.toString()
Converts a dictionary to a string.

```cs
{"1": 1, 1: "1"}.toString(); // '{"1": 1, 1: "1"}'
{"1": {1: "1", "1": 1}, 1: "1"}.toString(); // '{"1": {"1": 1, 1: "1"}, 1: "1"}'
```

### dict.toBool()

Converts a dictionary to a boolean. A dictionary is a "truthy" value when it has a length greater than 0.

```cs
var x = {};

x.toBool(); // false
x["test"] = 1;
x.toBool(); // true
```

### dict.len()

Returns the length of the given dictionary.

```cs
{1: "one", 2: "two", 3: "three"}.len(); // 3
```

### dict.exists(string)

To check if a key exists within a dictionary use `.exists()`

```cs
var myDict = {"key": 1, "key1": true};
var keyExists = myDict.exists("key"); // true
var keyDoesNotExist = myDict.exists("unknown key"); // false
```

### dict.remove(string)

To remove a key from a dictionary use `.remove()`.

**Note**: If you try to remove a key that does not exist a runtime error is raised, use together with `.exists()`.

```cs
var myDict = {"key": 1, "key1": true};
myDict.remove("key");
myDict; // {'key1': true}
myDict.remove("unknown key"); // [line 1] in script: Key 'unknown key' passed to remove() does not exist
```

### Copying dictionaries

This is the exact same scenario as lists, so refer to [copying Lists](#copying-lists) for information as to what is happening.

```cs
var myDict = {"key": {"test": 10}};
var myDict1 = myDict.copy(); // Shallow copy
var myDict2 = myDict.deepCopy(); // Deep copy
```

### dict.forEach(func)

To run a function on every element in a dictionary we use `.forEach`. The callback function
passed to `.forEach` expects two parameters which will be the key-value pair of the dictionary.

```cs
const myDict = {"key": 1, "key1": 2};

myDict.forEach(def (key, value) => {
    print("Key: {} Value: {}".format(key, value));
});
```

### dict.merge(anotherDict)

To merge with another dictionary. This operation will produce a new object. If another dictionary contains a key that exists in invoking one, the invoking dictionary's key will be taken. 

```cs
const dictOne = {"key": 1, "key1": 2, "key2": 3};
const dictTwo = {"key3": 4,"key1":0};

const mergedDict=dictOne.merge(dictTwo);

mergedDict; //{"key2": 3, "key": 1, "key3": 4, "key1": 2}
```
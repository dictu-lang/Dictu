---
layout: default
title: Object
nav_order: 16
parent: Standard Library
---

# Object
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Object

To make use of the Object module an import is required.

```js
import Object;
```

### Object.getClassRef(String) -> Result\<Object>

This method will attempt to get a class reference from the class name provided as a string.

Returns a Result and unwraps to an Object upon success.

**NOTE:** This currently only works if the class is defined in the global scope

```cs
class Test {}

Object.getClassRef("Test").unwrap(); // <Cls Test>
```

### Object.createFrom(String) -> Result\<Object>

This method will attempt to create a new object from the class name provided as a string.

Returns a Result and unwraps to an Object upon success.

**NOTE:** This currently only works if the class is defined in the global scope

```cs
class Test {}

Object.createFrom("Test").unwrap(); // <Test instance>
```

### Object.hash(Value) -> String

This method will return a string of the object's hash value.

**NOTE** Strings, Booleans, Nil will always return the same value since these values are interned.

```cs
Object.hash("Dictu");
```

### Object.prettyPrint(Value, Number: indent -> Optional)

This method will output to stdout a string representation of the given value.

**NOTE** Strings, dicts, lists, numbers, booleans, and nil are valid values for pretty printing at this time.

```cs
Object.prettyPrint([1, 2, 3]);

// Output
'[1, 2, 3]'
```

```cs
Object.prettyPrint({"a": 1}, 4);

// Output
'
{
    "a": 1
}
'
```

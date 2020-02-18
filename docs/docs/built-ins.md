---
layout: default
title: Built in functions
nav_order: 8
---

# Built in functions
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Built in functions

Global functions which are built into Dictu.

### print(...values...)

Prints a given list of values to stdout.

```js
print(10); // 10
print("test"); // "test"
print(10, "test", nil, true); // 10, "test", nil, true
```

### input(string: prompt -> optional)

Gathers user input from stdin and returns the value as a string. `input()` has an optional prompt which will be shown to
the user before they enter their string.

```js
input();
input("Input: ");
```

### type(value)

Returns the type of a given value as a string.

```js
type(10); // "number"
type(true); // "bool"
type([]); // "list"
```

### len(string OR collection)

Returns the length of a given string or collection (list, dictionary, set).

```js
len("Hello"); // 5
len([1, 2]); // 2
len({"test": 10}); // 1
```

### bool(value)

Converts a given value to the boolean value.

```js
bool(0); // false
bool(""); // false
bool(1); // true
bool("some string"); // true
```

### str(value)

Converts a given value to a string.

```js
str(10.2); // "10.2"
str([1, 2]); "[1, 2]"
str({"hi": 10}); '{"hi": 10}'
```

### number(string)

Converts a string to number.

```js
number("10.2"); // 10.2
number("10"); // 10
```

### assert(boolean)

Raise a runtime error if the given boolean is not true.

```js
assert(10 > 9);
assert(9 > 10); // assert() was false!
```
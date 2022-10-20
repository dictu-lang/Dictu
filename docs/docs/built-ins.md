---
layout: default
title: Built-in
nav_order: 9
---

# Built-in
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Built-in variables

### \__file__

The path name of the compilation unit.

## Built-in functions

Global functions which are built into Dictu.

### print(...values)

Prints a given list of values to stdout.

```cs
print(10); // 10
print("test"); // "test"
print(10, "test", nil, true); // 10, "test", nil, true
```

### printError(...values)

Prints a given list of values to stderr.

```cs
printError(10); // 10
printError("test"); // "test"
printError(10, "test", nil, true); // 10, "test", nil, true
```

### input(string: prompt -> optional)

Gathers user input from stdin and returns the value as a string. `input()` has an optional prompt which will be shown to
the user before they enter their string.

```cs
input();
input("Input: ");
```

### type(value)

Returns the type of a given value as a string.

```cs
type(10); // "number"
type(true); // "bool"
type([]); // "list"
```

### assert(boolean)

Raise a runtime error if the given boolean is not true.

```cs
assert(10 > 9);
assert(9 > 10); // assert() was false!
```

### isDefined(string)

Returns a boolean depending on whether a variable has been defined in the global scope.

```cs
isDefined("isDefined"); // true
isDefined("garbage value"); // false
```

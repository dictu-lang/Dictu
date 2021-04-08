---
layout: default
title: Variables and types
nav_order: 3
---

# Variables and types
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## Data types

| Type | Example | Note |
| --- | --- | --- |
| String | `'foo'`, `"bar"` | You can use single or double quotes to represent strings in Dictu. |
| Number | `100`, `100.5` | This data type includes both integers (whole numbers) and floats (numbers with decimals). |
| Boolean | `true`, `false` | Fun fact: Boolean data types are named after George Boole. |
| List | `[1, 2, ‘hi’, true, nil]` | Lists can contain any data type or combination of data types. |
| Dictionary | `{"key1": 10, 2: "two", true: "true", nil: "nil"}` | Dictionaries have key-value pairs, like a dictionary (Word: Definition). Keys must be an immutable data type (strings, numbers, nil or boolean). Values can be any data type. |
| Set | `set("1", 1, "1")` | Sets are unordered collections of immutable, unique values. You can create a set with the `set()` function. |
| Nil | nil | Used to signify no value (much like null in other languages) |
| Result | Success, Error | See [Error Handling section.](/docs/error-handling) |

## Declaring a variable

```cs
var someNumber = 10;
var someString = "hello";
var someOtherString = 'hello';
var someBoolean = true;
var someFalseBoolean = false;
var someNilValue = nil;
var someOtherNilValue; // This variable is equal to nil
var someList = [1, 2, "hi", 'hi', nil, true, false];
var someDict = {"key": 10, "key1": true, 2: [1, "2", false], true: {"1": 2}};
var someSet = set();
```

Variable names can start with an uppercase or lowercase letter or an underscore. The rest of the characters can be letters, digits, or underscores. The value must be another defined variable or a valid data type. Variables cannot be named several reserved terms: `import`, `true`, `false`, `nil`, `const`, `var`, and more.

Once a variable has been defined, `var` is no longer needed to update the value of the variable:
```cs
var someNumber = 10;
someNumber = someNumber + 13;
```

Variables can also change their data type without being redeclared with `var`:
```cs
var someVariable = 10;
someVariable = "A string"; // Perfectly valid!
```

You can also define multiple variables with a single statement.
```cs
var a,
    b = 10,
    c = "Hello!";
print(a, b, c); // nil, 10, 'Hello!'
```

### Constants

Constants are like variables, however they are read-only. Once a constant has been declared, its value cannot be changed later in the program. Constant names must follow the same rules as variable names.

```cs
const x = 10;
x = 15; // [line 1] Error at '=': Cannot assign to a constant.
print(x); // 10
```

Like variables, multiple constants can be declared at once:
```cs
const y = 10,
      z = 20; // Both y and z are constants
print(y, z); // 10, 20
```

### List unpacking

List unpacking is a feature in which you can define multiple variables at a given time from values within a list.
To unpack the amount of variables being defined **must** match the amount of values within the list otherwise a runtime error will occur.
```js
// Perfectly valid!
var [a, b, c] = [1, 2, 3];
print(a); // 1
print(b); // 2
print(c); // 3

// Perfectly valid!
const [x, y, z] = [1, 2, 3];
print(x); // 1
print(y); // 2
print(z); // 3

// Perfectly valid!
var myList = [1, 2, 3];
var [val1, val2, val3] = myList;
print(val1); // 1
print(val2); // 2
print(val3); // 3

// Error!
var [d] = [1, 2];
var [e, f] = [1];
```
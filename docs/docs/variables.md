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
| String | ‘foo’, “bar” | You can use single or double quotes to represent strings in Dictu. |
| Number | 100, 100.5 | This data type includes both integers (whole numbers) and floats (numbers with decimals). |
| Boolean | true, false | Fun fact: Boolean data types are named after Robert Boole. |
| List | [1, 2, ‘hi’, true, nil] | Lists can contain any data type or combination of data types. |
| Dictionary | {“key1”: 10, 2: “two”, true: “true”, nil: “nil”} | Dictionaries have key-value pairs, like a dictionary (Word: Definition). Values can be any of the data types above except lists. |
| Set | `set("1", 1, "1")` will actually mean `{"1", 1}` | Sets are unordered collections of hashable, unique values. You can create a set with the `set()` function, which returns a set. |
| Nil | nil | Don’t confuse `nil` with `null`! While they mean the same thing in the English language, and the latter is more common in programming, `null` has no significance in Dictu. |

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

Variables can also change their data type without being redclared with `var`:
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

## Type casting

You can convert between data types using several built-in methods.

### value.toString();
You can convert values into strings with the toString() method. This method can be called on variables, constants, and directly on values, as shown below.
```cs
true.toString(); // 'true'
nil.toString(); // 'nil'
1.toString(); // '1'
{"key": "value"}.toString(); // {"key": "value"}
[1,2,3].toString(); // '[1, 2, 3]'
set().toString(); // '{}'
```

### value.toBool();
You can convert values into booleans with the `toBool()` method. Dictu will decide whether a value is "truthy" or not. A "truthy" value is longer than 0 or is not 0 when checking number types. `nil` is always false.

```cs
"".toBool(); // false
" ".toBool(); // true
"test".toBool(); // true
"false".toBool(); // true

0.toBool(); // false
10.toBool(); // true

[].toBool(); // false
[1].toBool(); // true
[[]].toBool(); // true
```

### value.toNumber();

The `toNumber()` method is only for converting a string to a number. This method has a few strange caveats (see the examples below).

```cs
"123".toNumber(); // 123
" 123".toNumber(); // 123
"123 ".toNumber(); // nil
"Hello".toNumber(); // nil
```
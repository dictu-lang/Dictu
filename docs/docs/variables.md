---
layout: default
title: Variables
nav_order: 3
---

# Variables
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## Data types

| Type         | Values                                                                                                    |
|:-------------|:----------------------------------------------------------------------------------------------------------|
| Number       | This includes both integers and floats. <br/><br/>10, 10.5                                                |
| String       | "Hello World!", 'Hello World!'                                                                            |
| Boolean      | true, false                                                                                               |
| List         | Lists can contain any value. <br/><br/>[1, 2, "hi", true, nil]                                            |
| Dictionary   | Dictionaries are key value pairs. Currently keys have to be strings. <br/><br/>{"key1": 10, "key2": true} |
| nil          | nil                                                                                                       |

## Declaring a variable

Syntax:
```js
var someNumber = 10;
var someString = "hello";
var someOtherString = 'hello';
var someBoolean = true;
var someFalseBoolean = false;
var someNilValue = nil;
var someOtherNilValue; // Variables declared without a value are set to nil
var someList = [1, 2, "hi", 'hi', nil, true, false]; // Lists can hold any value
var someDict = {"key": 10, "key1": true, "key2": [1, "2", false], "key3": {"1": 2}}; // Dictionaries must have a string key, but can hold any value
```


To define a variable in Dictu the syntax is as follows `var <identifier> = <value>;` where identifier
is defined as such `[a-zA-Z_][a-zA-Z_]*` and value is a valid datatype.

Once a variable has been defined, `var` is no longer needed to update the value of the variable.

E.g
```js
var someNumber = 10;

someNumber = someNumber + 1;
```

Variables are also allowed to change their type at runtime.

E.g
```js
var someVariable = 10;
someVariable = "Some text"; // Is perfectly valid
```

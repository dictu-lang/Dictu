---
layout: default
title: Built in functions
nav_order: 9
---

# Built in functions

| Function | Description                                                   | Example                                        |
|:---------|:--------------------------------------------------------------|:-----------------------------------------------|
| print    | Prints arguments to the console                               | `print(10, "test", nil, true);`                |
| input    | Returns user input as a string                                | `input(); input("Input: ");`                   |
| type     | Returns a string of the type of variable.                     | `type(x);`                                     |
| bool     | Returns a boolean based on the value passed in                | `bool(""); bool("true");`                      |
| str      | Convert a value to a string                                   | `str(10.2); str([1, 2]); str({"hi": 10})`      |
| number   | Convert string to number value                                | `number("10.2");`                              |
| assert   | Runtime error raised if condition doesn't evaluate to true    | `assert(true);`                                |
| len      | Returns the length of a string or collection                  | `len("Hello"); len([1, 2]); len({"test": 10});`|

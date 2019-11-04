---
layout: default
title: Built in functions
nav_order: 9
---

# Built in functions

| Function | Description                                                   | Example                        |
|:---------|:--------------------------------------------------------------|:-------------------------------|
| print    | Prints arguments to the console                               | print(10, "test", nil, true);  |
| input    | Returns user input as a string                                | input(); input("Input: ");     |
| type     | Returns a string of the type of variable.                     | type(x);                       |
| bool     | Returns a boolean based on the value passed in                | bool(""); // false<br> bool("true"); // true |
| str      | Convert a value to a string                                   | str(10.2); str([1, 2]); str({"hi": 10}) |
| number   | Convert string to number value                                | number("10.2");                |
| time     | Returns UNIX timestamp                                        | time();                        |
| clock     | Returns number of clock ticks since the start of the program | clock();                        |
| sleep    | Pauses execution for given time in seconds                    | sleep(10);                     |
| assert   | Runtime error raised if condition doesn't evaluate to true    | assert(true);                  |
| len      | Returns the length of a string or collection                  | len("Hello"); len([1, 2]); len({"test": 10});   |
| min      | Returns the smallest number of the arguments                  | min(1, 2, 3); min([1, 2, 3]);  |
| max      | Returns the largest number of the arguments                   | max(1, 2, 3); max([1, 2, 3]);  |
| average  | Returns an average number of the arguments                    | average(1, 2, 3); average([1, 2, 3]); |
| sum      | Returns a sum of all arguments passed                         | sum(1, 2, 3); sum([1, 2, 3]);  |
| floor    | Returns largest integer less than or equal to given input.    | floor(17.4);                   |
| round    | Returns the nearest integer                                   | round(17.4);                   |
| ceil     | Returns smallest integer greater than or equal to given input.| ceil(17.4);                    |
| abs      | Returns absolute value                                        | abs(-10);                      |

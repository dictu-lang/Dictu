---
layout: default
title: Control Flow
nav_order: 7
---

# Control Flow
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## If statement

```js
if (false or true) {
    var variable = "Hello";
    print(variable);  // Print is a native function so parenthesis are required
}

// Single line if statement body does not require braces
if (true)
    print("Hello");
    
if (false)
    print("Not going to print!");
else
    print("Will print!");
    
    
var x = 5;
if (x == 6) {
    print("Not going to print!");
} else if (x == 5) {
    print("Will print!");
} else {
    print("Not going to print!");
}
```
## While loop

```js
// While loop
var i = 0;
while (i < 10) {
    print(i);
    ++i;
}
```

Dictu also implements syntactic sugar for a while true loop.

```js
// While 'true' loop
// If no expression is made, true is implicitly assumed
while {
    print("Hello");
    sleep(1);
}
```

## For loop

```js
// For loop
for (var i = 0; i < 10; ++i) {
    print(i);
}
```

## Functions

```python
//Define the function 
def someFunction() {
    print("Hello!");
}

someFunction(); // Call the function

def someFunctionWithArgs(arg1, arg2) {
    print(arg1 + arg2);
}

someFunctionWithArgs(5, 5); // 10
someFunctionWithArgs("test", "test"); // testtest

def someFunctionWithReturn() {
    return 10;
}

print(someFunctionWithReturn()); // 10
```

`def` is a keyword in Dictu which defines a function. `def` needs to be followed by the function name and then the function parameters. The body of the function is then enclosed within braces.

Variables defined in a function are local to the scope of the function.

```python
def someFunction() {
    var myVariable = 10;
    print(myVariable);
}

someFunction();

myVariable = 11;
```
Output
```
10
[line 8] in script: Undefined variable 'myVariable'.
```

Functions can also be stored in variables, or within data structures such as lists.

```python
def someFunction() {
    print("Hello!");
}

var functionVariable = someFunction;

functionVariable(); // Hello!

def someOtherFunction(parameter) {
    print(parameter);
}

var someList = [someOtherFunction, "Hello!"];

someList[0](someList[1]); // Hello!
``` 
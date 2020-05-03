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
## Loops
### While loop

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

### For loop

```js
// For loop
for (var i = 0; i < 10; ++i) {
    print(i);
}
```

### Continue statement

Continue allows execution of a loop to restart prematurely.

```js
// For loop
for (var i = 0; i < 10; ++i) {
    if (i % 2 == 0)
        continue; // Skip all even numbers

    print(i); // Only odd numbers will be printed
}
```

### Break statement

Break allows execution of a loop to stop prematurely.

```js
// For loop
for (var i = 0; i < 10; ++i) {
    if (i > 5)
        break; // Exit the loop here

    print(i); // Only numbers less than or equal to 5 will be printed
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

Functions can also have optional parameters. If a value is not passed, the parameter will take the default value
given in the function header. Note: non-optional parameters **must** be defined before optional parameters.

```python
def someFunction(a, b=10) {
    print(a, b);
}

someFunction(1, 2); // 1, 2
someFunction(1); // 1, 10

def someFunction(a, b="default!") {
    print(a, b);
}

someFunction(1, 2); // 1, 2
someFunction(1); // 1, "default!"
```

### Arrow functions

Arrow functions are anonymous functions which are a syntactically compact version of a regular function.
Arrow functions can either return a single expression, or can act like a regular function depending
on whether the body is enclosed in {} or not.

```
// If the arrow functions body is not enclosed in {} then a single
// expression is allowed and the value is returned implicitly without the use of "return".

var func = def () => 10;
func(); // 10

// If a function body is enclosed by {} then it acts like a regular function.

var func = def() => {return 10;};
func(); // 10
```

Arrow functions can also use optional parameters.

```
var func = def(a=10) => a;
func(); // 10
func(100); // 100

var func = def(a=10) => {return a;};
func(); // 10
func(100); // 100
```
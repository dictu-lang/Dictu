---
layout: default
title: Control flow
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

```cs
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

## Switch Statement

A switch statement can be seen as a more maintainable if/else if chain. It evaluates an expression and then enters a 
case block depending upon if the expression matches the expression of the case block.

The value supplied to a case block can be of any type and follows the same rules of equality as the `==` operator.

```cs
switch (10) {
    case 1: {
        // 10 doesn't equal 1, so this is skipped.
    }

    case 10: {
        // Match! This block of code is executed!
    }
}
```

### Default

Sometimes there may be a need for a bit of code to be executed if none of the case blocks match the expression of the switch statement. This is where
the default block comes into play.

```
switch ("string") {
    case "nope": {

    }

    case "nope again!": {

    }

    default: {
        // This will get executed!
    }
}
```

### Multi-Value Case

Sometimes we need to execute a block of code based on a set of values. This is easily done by just supplying a comma separated list
of values to the case block.

```cs
switch (10) {
    case 1, 3, 5, 7, 9: {
        // 10 doesn't equal any of the supplied values, this is skipped.
    }

    case 0, 2, 4, 6, 8, 10: {
        // Match! This block of code is executed!
    }
}
```

## Loops
### While loop

```cs
// While loop
var i = 0;
while (i < 10) {
    print(i);
    i += 1;
}
```

Dictu also implements syntactic sugar for a while true loop.

```cs
// While 'true' loop
// If no expression is made, true is implicitly assumed
while {
    print("Hello");
    sleep(1);
}
```

### For loop

```cs
// For loop
for (var i = 0; i < 10; i += 1) {
    print(i);
}
```

### Continue statement

Continue allows execution of a loop to restart prematurely.

```cs
// For loop
for (var i = 0; i < 10; i += 1) {
    if (i % 2 == 0)
        continue; // Skip all even numbers

    print(i); // Only odd numbers will be printed
}
```

### Break statement

Break allows execution of a loop to stop prematurely.

```cs
// For loop
for (var i = 0; i < 10; i += 1) {
    if (i > 5)
        break; // Exit the loop here

    print(i); // Only numbers less than or equal to 5 will be printed
}
```

## Functions

```cs
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

```cs
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

```cs
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

```cs
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
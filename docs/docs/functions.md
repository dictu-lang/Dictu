---
layout: default
title: Functions
nav_order: 8
---

# Functions
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Regular Functions

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

## Arrow Functions

Arrow functions are anonymous functions which are a syntactically compact version of a regular function.
Arrow functions can either return a single expression, or can act like a regular function depending
on whether the body is enclosed in {} or not.

```cs
// If the arrow functions body is not enclosed in {} then a single
// expression is allowed and the value is returned implicitly without the use of "return".

var func = def () => 10;
func(); // 10

// If a function body is enclosed by {} then it acts like a regular function.

var func = def() => {return 10;};
func(); // 10
```

Arrow functions can also use optional parameters.

```cs
var func = def(a=10) => a;
func(); // 10
func(100); // 100

var func = def(a=10) => {return a;};
func(); // 10
func(100); // 100
```

## Variadic Functions

Variadic functions are simply functions which accepts a variable number of arguments.
In Dictu, variadic functions typically have a spread formal parameter which can accept multiple arguments when the function is called.
Also, the spread parameter must come last in the function parameter list.

Here's an example:

```cs
def someFunction(...args) {
    print(args);
}

functionVariable('hello'); // ['hello']
functionVariable(1, 2, 3, 4); // [1, 2, 3, 4]

def someOtherFunction(parameter, ...others) {
    print(parameter, others);
}

someOtherFunction('hello'); 
// hello
// []
someOtherFunction(1, 2, 3, 4); 
// 1
// [2, 3, 4]
```

Arrow functions can also be variadic:

```cs
var func = def(...args) => print(args);
func(); // []
func(100, 200, 'x'); // [100, 200, 'x']

var func = def(a, ...b) => print(a, args);
func('x'); 
// x
// []
func('x', 100); 
// x
// [100]
```

It is important to note that class constructors cannot be variadic. Also, variadic functions can have no default or optional parameters.
For example, the following would not work:

```cs
// fails
class BadClass {
    // Constructor
    init(...message) {
        print(message);
    }
}

// fails
def badFunction(a=5, ...x) {
    // ...
}
```
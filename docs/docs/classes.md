---
layout: default
title: Classes
nav_order: 8
---

# Classes
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Classes

Classes provide a means to gather functions and data together to provide a blueprint for objects.

## Defining a class

```js
class SomeClass {
    // Constructor
    init() {
        print("Object created!");
    }
}

SomeClass(); // Object created!
```

## Constructor

`init()` is the method name for a constructor in Dictu. A constructor is a method that is called when an object is instantiated. Instantiating an object, is just like invoking a function, except you "invoke" the class. You can also pass arguments to the constructor to be used.

```js
class SomeClass {
    // Constructor
    init(message) {
        print(message);
    }
}

SomeClass("Object created!"); // Object created!
```

## Methods

Methods are functions defined within a class. When defining a method in Dictu, the `def` keyword is not used and instead its just the method name and parameter list.

```js
class SomeClass {
    // Method
    printMessage() {
        print("Hello!");
    }
}

SomeClass().printMessage(); // Hello!
```

## This

`this` is a variable which is passed to all methods which are not marked as static. `this` is a reference to the object you are currently accessing. `this` allows you to modify instance variables of a particular object.

```js
class SomeClass {
    // Constructor
    init(message) {
        this.message = message;
    }

    printMessage() {
        print(this.message);
    }
}

var myObject = SomeClass("Some text!");
myObject.printMessage(); // Some text!
```

## Static methods

Static methods are methods which do not reference an object, and instead belong to a class. If a method is marked as static, `this` is not passed to the object. This means static methods can be invoked without instantiating an object.

```js
class SomeOtherClass {
    init(someArg) {
        this.someArg = someArg;
    }
    
    // 'this' is not passed to static methods
    static printHello() {
        print("Hello");
    }

    printMessage() {
        print("Some Text!");
    }
}

SomeOtherClass.printHello(); // Hello
SomeOtherClass().printMessage(); // Some Text
```
Output
```js
Hello
[line 17] in script: 'printMessage' is not static. Only static methods can be invoked directly from a class.
```

## Inheritance

```js
class BaseClass {
    init() {
        this.someVariable = "Hello!";
    }

    printMessage(message) {
        print(message);
    }
}

class NewClass < BaseClass {
    init() {
        super.init();
    }
}

var obj = NewClass();
obj.printMessage("Hello!"); // Hello!
print(obj.someVariable); // Hello!
```

The syntax for class inheritance is as follows: `class DerivedClass < BaseClass`. `super` is a variable that is reference to the class that is being inherited.
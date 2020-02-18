---
layout: default
title: Classes
nav_order: 9
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

## Attributes

Attributes in Dictu are instance attributes, and these attributes get defined either inside the methods or on the method directly. There is no concept of attribute access modifiers in python and attributes
are available directly from the object without the need for getters or setters.

```js
class Test {
    init() {
        this.x = 10;
    }
}

var myObject = Test();
print(myObject.x); // 10
```

### hasAttribute

Attempting to access an attribute of an object that does not exist will throw a runtime error, and instead before accessing, you should check
if the object has the given attribute. This is done via `hasAttribute`.

```js
class Test {
    init() {
        this.x = 10;
    }
}

var myObject = Test();
print(myObject.hasAttribute("x")); // true
print(myObject.hasAttribute("y")); // false

print(myObject.z); // Undefined property 'z'.
```

### getAttribute

Sometimes in Dictu we may wish to access an attribute of an object without knowing the attribute until runtime. We can do this via the `getAttribute` method.
This method takes a string and an optional default value and returns either the attribute value or the default value (if there is no attribute and no default value, nil is returned).

```js
class Test {
    init() {
        this.x = 10;
    }
}

var myObject = Test();
print(myObject.getAttribute("x")); // 10
print(myObject.getAttribute("x", 100)); // 10
print(myObject.getAttribute("y", 100)); // 100
print(myObject.getAttribute("y")); // nil
```

### setAttribute

Similar concept to `getAttribute` however this allows us to set an attribute on an instance.

```js
class Test {
    init() {
        this.x = 10;
    }
}

var myObject = Test();
myObject.setAttribute("x", 100);
print(myObject.x); // 100
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

SomeOtherClass.printHello();
SomeOtherClass.printMessage();
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

## Traits

Dictu only allows inheritance from a single parent class, which can cause complications when we need functionality from more than one class.
This is where traits come into play. A trait is like a class, in the fact it has methods, and can deal with object attributes however, differ in the fact
a trait can not be instantiated on its own.

```js
trait MyTrait {
    hello() {
        print("Hello {}".format(this.name));
    }
}

class MyClass {
    use MyTrait;

    init(name) {
        this.name = name;
    }
}

var myObject = MyClass("Jason");
myObject.hello(); // Hello Jason

MyTrait(); // Runtime error: Can only call functions and classes.
```

Sometimes we will have multiple traits, each with slightly different functionality, but we need
functionality from all of these traits, in this instance, we can just use more than one trait.

```js
trait MyTrait {
    hello() {
        print("Hello {}".format(this.name));
    }
}

trait MyOtherTrait {
    test() {
        print("Test!");
    }
}

class MyClass {
    use MyTrait, MyOtherTrait;

    init(name) {
        this.name = name;
    }
}

var myObject = MyClass("Jason");
myObject.hello(); // Hello Jason
myObject.test(); // Test!
```

Traits also do not suffer from the diamond problem unlike multiple inheritance, instead if two traits
are used and they have the same method, the last most used trait has precedence. This means the order
of trait inclusion into a class is important.

```js
trait MyTrait {
    hello() {
        print("Hello {}".format(this.name));
    }
}

trait MyOtherTrait {
    hello() {
        print("This will not be ran!");
    }
}

class MyClass {
    use MyOtherTrait, MyTrait; // Order is important

    init(name) {
        this.name = name;
    }
}

var myObject = MyClass("Jason");
myObject.hello(); // Hello Jason
```
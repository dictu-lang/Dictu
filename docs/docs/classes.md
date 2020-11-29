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

```cs
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

```cs
class SomeClass {
    // Constructor
    init(message) {
        print(message);
    }
}

SomeClass("Object created!"); // Object created!
```

### Implicit properties

Dictu actually has a way to define properties on the object without explicitly setting each variable passed into the constructor on the object through `this`.

```cs
class SomeClass {
    // The var keyword here makes the argument passed in be set as an instance variable
    init(var a, var b) {}
}

var obj = SomeClass(10, 20);
print("{} {}".format(obj.a, obj.b)); // "10 20"
```

The `var` keyword is optional on the constructor parameters, and can be in any order.

```cs
class SomeClass {
    init(var a, b, c, var d) {
        // b and c are not set as instance properties
    }
}

var obj = SomeClass(10, 20, 30, 40);
print("{} {} {} {}".format(
    obj.getAttribute("a"),
    obj.getAttribute("b"),
    obj.getAttribute("c"),
    obj.getAttribute("d")
)); // "10 nil nil 40"
```

## Methods

Methods are functions defined within a class. When defining a method in Dictu, the `def` keyword is not used and instead its just the method name and parameter list.

```cs
class SomeClass {
    // Method
    printMessage() {
        print("Hello!");
    }
}

SomeClass().printMessage(); // Hello!
```

### toString

Classes and instances can both be converted to a string using the toString method. If you want a different string
representation for an object you can overload the toString method in your class.

```cs
class Test {}

class TestOverload {
    init() {
        this.name = "Testing";
    }

    toString() {
        return "{} object".format(this.name);
    }
}

print(Test.toString()); // '<cls Test>'
print(Test().toString()); // '<Test instance>'

print(TestOverload.toString()); // '<cls TestOverload>'
print(TestOverload().toString()); // 'Testing object'

```

## This

`this` is a variable which is passed to all methods which are not marked as static. `this` is a reference to the object you are currently accessing. `this` allows you to modify instance variables of a particular object.

```cs
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

```cs
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

```cs
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

```cs
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

```cs
class Test {
    init() {
        this.x = 10;
    }
}

var myObject = Test();
myObject.setAttribute("x", 100);
print(myObject.x); // 100
```

### Optional Chaining

Optional chaining allows you to read a property or method of an instance without explicitly having to check for `nil` before
attempting to access.

**Note:** If the left hand value is not nil the property / method **must** still exist when attempting to access otherwise a runtime error will occur.

```js
class Test {
    // Lets say you have a method that 
    // could return an object or nil
    someMethod() {
        return nil;
    }
    
    someOtherMethod() {
        print("method!");
    }
}

// Here there is no explicit nil check.
print(Test().someMethod()?.someOtherMethod()); // nil

// If the operand is not nil the method / property must exist  
print(Test()?.unknownMethod()); // Undefined property 'unknownMethod'.
``` 

## Class variables

A class variable, is a variable that is defined on the class and not the instance. This means that all instances of the class will have access
to the class variable, and it is also shared across all instances.

```cs
class SomeClass {
    var classVariable = 10; // This will be shared among all "SomeClass" instances

    init() {
        this.x = 10; // "x" is set on the instance
    }
}

print(SomeClass.classVaraible); // 10

var x = SomeClass();
var y = SomeClass();

print(x.classVariable); // 10
print(y.classVariable); // 10

SomeClass.classVaraible = 100;

print(x.classVariable); // 100
print(y.classVariable); // 100
```

## Static methods

Static methods are methods which do not reference an object, and instead belong to a class. If a method is marked as static, `this` is not passed to the object. This means static methods can be invoked without instantiating an object.

```cs
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
```cs
Hello
[line 17] in script: 'printMessage' is not static. Only static methods can be invoked directly from a class.
```

## Inheritance

```cs
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

## Abstract classes

An abstract class is a base class that can not be instantiated, like a trait, however is much like a contract in that it defines methods that need to be implemented
within a class. An abstract class can have methods which implement the body, and would work like a normal class being inherited, however, if it includes methods which
have been marked as abstract, it enforces the inheriting class to implement these methods.

```cs
abstract class AbstractClass {
    // We do not define the body of an abstract method
    abstract test()
    
    // We can also provide methods with the body that will be inherited as normal
    anotherFunc() {
        print("Func!");
    }
}

// If we left the class as is, a runtime error would occur.
// Class Test does not implement abstract method test
class Test < AbstractClass {}

class Test < AbstractClass {
    // We have implemented the abstract method, and therefore, satisfied the abstract class
    test() {
        print("Test!");
    }
}
```

## Traits

Dictu only allows inheritance from a single parent class, which can cause complications when we need functionality from more than one class.
This is where traits come into play. A trait is like a class, in the fact it has methods, and can deal with object attributes however, differ in the fact
a trait can not be instantiated on its own.

```cs
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

```cs
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

```cs
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

## Copying objects

Class instances are a mutable type, this means if you were to take the reference of an object and use it in a new variable
and you mutate the instance in the new variable, it would mutate the object at the old variable, since its a reference to the same object.

```cs
class Test {
    init() {
        this.x = 10;
    }
}

var myObject = Test();
var myNewObject = myObject;
myNewObject.x = 100;
print(myObject.x); // 100
```

To get around this, instances have two methods, obj.copy() and obj.deepCopy().

### obj.copy()

This method will take a shallow copy of the object, and create a new copy of the instance. Mutable types are still references
and will mutate on both new and old if changed. See obj.deepCopy() to avoid this.

```cs
class Test {
    init() {
        this.x = 10;
    }
}

var myObject = Test();
var myNewObject = myObject.copy();
myNewObject.x = 100;
print(myObject.x); // 10

myObject.obj = Test(); // Reference to a mutable datatype
myNewObject = myObject.copy();
myNewObject.obj.x = 100;
print(myObject.obj.x); // 100
```

### obj.deepCopy()

This method will take a deep copy of the object, and create a new copy of the instance. The difference with deepCopy()
is if the object contains references to any mutable datatypes these will also be copied and returned as new values meaning,
they will not be mutated on the old object.

```cs
class Test {
    init() {
        this.x = 10;
    }
}

var myObject = Test();
var myNewObject = myObject.deepCopy();
myNewObject.x = 100;
print(myObject.x); // 10

myObject.obj = Test(); // Reference to a mutable datatype
myNewObject = myObject.deepCopy();
myNewObject.obj.x = 100;
print(myObject.obj.x); // 10
```

## Checking instance types

### instance.isInstance(class)

Checking if an instance is of a given class is made very simple with the `isInstance` method. This method takes in a class as an 
argument and returns a boolean based on whether or not the object was instantiated from the given class. Since classes can inherit other
classes, and we know subclasses have the type of their parent class, the same holds true for `isInstance()`. If the instance being checked
is passed it's parent class as an argument `isInstance()` will evaluate to true.

```cs
class Test {}

var obj = Test();

print(obj.isInstance(Test)); // true

// Inheritance

class Test {}
class AnotherTest < Test {}

var testObj = Test();
var anotherTestObj = AnotherTest();

testObj.isInstance(AnotherTest); // false
anotherTestObj.isInstance(AnotherTest); // true
anotherTestObj.isInstance(Test); // true
```
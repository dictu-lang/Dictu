---
layout: default
title: Classes
nav_order: 10
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

## Access Levels
A big part of the OOP paradigm is encapsulation, the fact that the outside interface only shows what it needs to show and things
internal to the class do not need to be known to the user of the interface. Dictu manages this concept much the same as other languages 
through access levels. Unlike other languages, Dictu only has two access levels, `public` or `private`. If a method or instance variable
is marked as `private` it is only accessible from within the class itself. To mark an instance variable as private it can be done in two
ways, via [implicit attributes](#implicit-attributes) or by marking the attribute as private (note this must be done before defining the
attribute otherwise it will be publicly visible). To mark a method as private, preface the name with `private`.

```js
class SomeClass {
    private x;
    
    init(x) {
        this.x = x;
    }

    private getX() {
        return this.x;
    }
}

print(SomeClass(10).getX()); // 10
print(SomeClass(10).x); // Cannot access private attribute 'x' on 'SomeClass' instance.
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

### Implicit Attributes

Dictu actually has a way to define attributes on the object without explicitly setting each variable passed into the constructor on the object through `this`.
If `var` is used the instance variable has public visibility, if `private` is used the instance variable has private visibility.

```cs
class SomeClass {
    // The var keyword here makes the argument passed in be set as an instance variable with public visibility
    init(var a, var b) {}
}

var obj = SomeClass(10, 20);
print("{} {}".format(obj.a, obj.b)); // "10 20"
```

The `var` or `private` keywords are optional on the constructor parameters, and can be in any order.

```cs
class SomeClass {
    init(var a, b, c, var d, private e) {
        // b and c are not set as instance attributes
        // e is set as a private attribute
    }
}

var obj = SomeClass(10, 20, 30, 40, 50);
print("{} {} {} {} {}".format(
    obj.getAttribute("a"),
    obj.getAttribute("b"),
    obj.getAttribute("c"),
    obj.getAttribute("d"),
    obj.getAttribute("e")
)); // "10 nil nil 40 nil"
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

### toString() -> String

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

### methods() -> List

Sometimes we need to programmatically access methods that are stored within a class, this can be aided through the use of `.methods()`. This
will return a list of strings, where the strings are the names of all public methods stored within a class.

Note: The order of the list is not the same order the methods are defined.

```cs
class Test {
    init() {

    }

    someOther() {

    }
}

print(Test.methods()); // ["someOther", "init"]
print(Test().methods()); // Works on instances too - ["someOther", "init"]
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

Attributes in Dictu are instance attributes, and these attributes get defined either inside the methods or on the object directly.

```cs
class Test {
    init() {
        this.x = 10;
    }
}

var myObject = Test();
print(myObject.x); // 10
```

### hasAttribute(String) -> Boolean

Attempting to access an attribute of an object that does not exist will throw a runtime error, and instead before accessing an attribute that may not be there, you should check
if the object has the given attribute. This is done via `hasAttribute`.

Note: Will only check attributes with public visibility.

```cs
class Test {
    init() {
        this.x = 10;
    }
}

var myObject = Test();
print(myObject.hasAttribute("x")); // true
print(myObject.hasAttribute("y")); // false

print(myObject.z); // Undefined attribute 'z'.
```

### getAttribute(String) -> Value

Sometimes in Dictu we may wish to access an attribute of an object without knowing the attribute until runtime. We can do this via the `getAttribute` method.
This method takes a string and an optional default value and returns either the attribute value or the default value (if there is no attribute and no default value, nil is returned).

Note: Will only retrieve attributes with public visibility.

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

### getAttributes() -> Dict

The `getAttributes` method returns all class variables / constants, public methods and public attributes.

```cs
class Test {
    init() {
        this.x = 10;
    }
}

var myObject = Test();
print(myObject.getAttributes()); // {"fields": ["_name"], "methods": ["init"], "attributes": ["_class", "x"]}
```

### setAttribute(String, Value)

Similar concept to `getAttribute` however this allows us to set an attribute on an instance.

Note: Will set a attribute with public visibility.

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

Optional chaining allows you to read a attribute or method of an instance without explicitly having to check for `nil` before
attempting to access.

**Note:** If the left hand value is not nil the attribute / method **must** still exist when attempting to access otherwise a runtime error will occur.

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

// If the operand is not nil the method / attribute must exist  
print(Test()?.unknownMethod()); // Undefined attribute 'unknownMethod'.
```

### _class

`_class` is a special attribute that is added to instances so that a reference to the class is kept on objects. This will be
useful for things like pulling class annotations from an object where it's class may be unknown until runtime.

```cs
class Test {}

print(Test()._class); // <Cls Test>
```

### _name

`_name` is a special attribute that is added to classes that returns a string representation of the class name.

```cs
class Test {}

print(Test.name); // Test
```

## Class variables

A class variable is a variable that is defined on the class and not the instance. This means that all instances of the class will have access
to the class variable, and it is also shared across all instances.

```cs
class SomeClass {
    var classVariable = 10; // This will be shared among all "SomeClass" instances

    init() {
        this.x = 10; // "x" is set on the instance
    }
}

print(SomeClass.classVariable); // 10

const x = SomeClass();
const y = SomeClass();

print(x.classVariable); // 10
print(y.classVariable); // 10

SomeClass.classVariable = 100;

print(x.classVariable); // 100
print(y.classVariable); // 100
```

## Class Constants

Exactly the same as [Class Variables](#class-variables) except that it comes with a runtime guarantee that it will not be modified.

```cs
class SomeClass {
    const classVariable = 10; // This will be shared among all "SomeClass" instances

    init() {
        this.x = 10; // "x" is set on the instance
    }
}

print(SomeClass.classVariable); // 10

const x = SomeClass();
const y = SomeClass();

print(x.classVariable); // 10
print(y.classVariable); // 10

SomeClass.classVariable = 100; // Cannot assign to class constant 'SomeClass.classVariable'.
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

The syntax for class inheritance is as follows: `class DerivedClass < BaseClass`. `super` is a variable that is reference to the class that is being inherited.

Note: private methods and instance variables are not inherited.

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

MyTrait(); // Runtime error: 'trait' is not callable
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

### obj.copy() -> value

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

### obj.deepCopy() -> value

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

### instance.isInstance(Class) -> Boolean

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

## Annotations

Annotations are metadata that are applied to classes, methods and class variables and constants that by themselves have no impact. They, however, can provide user defined changes at runtime.

```cs
@Annotation
class AnnotatedClass {

}
```

```cs
class ClassWithMethodAnnotation {
    @ClassVarAnnotation
    var someVariable = 10;

    // This will be available in `.fieldAnnotations`
    @ClassConstVariable
    const y = 10;

    init() {}

    @MethodAnnotation
    method() {}
}
```

Annotations are access via the `.classAnnotations`, `.methodAnnotations` and `.fieldAnnotations` attributes available on all classes.

For class annotations, the returned data structure returned is a dictionary with keys set to the names of the annotations and their values if present. If no value is provided to the annotation, the value associated with the key is set to `nil`. 

For method annotations, the returned data structure is also a dictionary, however the keys are the method names and the values are also dictionaries containing the annotation name and associated values. If no value is provided to the annotation, the value associated with the key is set to `nil`. 

For class variables and class constants, the returned data structure is also a dictionary, however the keys are the variable/constant names and the values are also dictionaries containing the annotation name and associated values. If no value is provided to the annotation, the value associated with the key is set to `nil`. 

```cs
print(AnnotatedClass.classAnnotations); // {"Annotation": nil}
```

```cs
print(ClassWithMethodAnnotation.methodAnnotations); // {"method": {"MethodAnnotation": nil}}
```

Annotations can also be supplied a value, however, the value must be of type: nil, boolean, number or string.

```cs
@Annotation("Some extra value!")
class AnnotatedClass {

}

print(AnnotatedClass.classAnnotations); // {"Annotation": "Some extra value!"}
```

Multiple annotations can be supplied to classes and methods.

```cs
@Annotation
@AnotherAnnotation(10)
@SomeOtherAnnotation
class AnnotatedClass {

    @MethodAnnotation
    @AnotherMethodAnnotation(10)
    @SomeOtherMethodAnnotation("another one")
    method() {}

}
```

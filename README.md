# Dictu

*What is Dictu?*  
Dictu is a very simple dynamically typed programming language 
built upon the [craftinginterpreters tutorial](http://www.craftinginterpreters.com/contents.html).

*What does Dictu mean?*  
Dictu means `simplistic` in Latin.

### Dictu language examples

*Variable declaration:*
```
var someNumber = 10;
var someString = "hello";
var someOtherString = 'hello';
var someBoolean = true;
var someFalseBoolean = false;
var someNilValue = nil;
```
*Operators*
```
var test = 1; // Variable declaration

test = test + 1; // Addition. Variable 'test': 2
test = test - 1; // Subtraction. Variable 'test': 1
++test; // Prefix incremenet. Variable 'test': 2
--test; // Prefix decrement. Variable 'test': 1
test = test * 10; // Multiplication. Variable 'test': 10
test = test / 2; // Division. Variable 'test': 5
```
*Comments*
```
// This is a single line comment

/*
this
is
a
multiline
comment
*/
```
*If statements*
``` 
if (false or true) {
    var variable = "hello";
    print variable;
}

// Single line if statement body does not require braces
if (true)
    print "hello";
```
*Conditional expressions*
```
var someVar = 10;
var someTestVar = 11;

if (someVar == 10 and someTestVar == 11)
    print "True!";
    
if (someVar == 9 or someTestVar == 11)
    print "True!";
    
if (someVar >= 9)
    print "True!";
    
if (someVar <= 9)
    print "False!";
    
if (someVar > 9)
    print "True!";
    
if (someVar < 9)
    print "False!";
    
var someBoolean = false;
someBoolean = !someBoolean;
```
*Loops*
```
// For loop
for (var i = 0; i < 10; ++i) {
    print i;
}

// While loop
var i = 0;
while (i < 10) {
    print i;
    ++i;
}

// While 'true' loop
// If no expression is made, true is implicitly assumed
while {
    print "hello";
}
```
*Functions*
```
//Define the function 
def someFunction() {
    print "Hello!";
}

someFunction(); // Call the function

def someFunctionWithArgs(arg1, arg2) {
    print arg1 + arg2;
}

someFunctionWithArgs(5, 5); // 10
someFunctionWithArgs("test", "test"); //testtest

def someFunctionWithReturn() {
    return 10;
}

print someFunctionWithReturn(); // 10
```
*Classes*
```
class SomeClass {
    // Constructor
    init() {
        print "Object created!";
    }
}

var myObject = SomeClass(); // Instantiate the class

class SomeNewClass {
    // Constructor
    init(someArg) {
        this.someArg = someArg;
    }
    
    // 'this' is passed implicitly to all methods
    printSomeArg() {
        print this.someArg;
    }
}

var myObject = SomeNewClass("Instance variables!!"); // Instantiate the class
myObject.printSomeArg(); // Call the method

class SomeOtherClass {
    init(someArg) {
        this.someArg = someArg;
    }
    
    // 'this' is not passed to static methods
    static printHello() {
        print "hello";
    }
}

SomeOtherClass.printHello(); // Static methods dont need the object to be instantiated 
```

## Running Dictu
```bash
$ git clone x
$ cd Dictu
$ make dictu
$ ./dictu test.du
```

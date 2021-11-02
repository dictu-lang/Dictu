---
layout: default
title: UnitTest
nav_order: 15
parent: Standard Library
---

# Unit Test
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## UnitTest
Unit testing is a very important part of software development and something we as developers
should always strive to complete. Dictu aims to make this slightly easier by having a unit test
framework built within the language.

The framework works by first inheriting from the abstract `UnitTest` class provided
to us within this module.

Note: If any tests fail then the suite will exit with an exit code of 1. This will allow CI/CD processes
to easily detect a failing test suite.

```cs
from UnitTest import UnitTest;

class Test < UnitTest {

}
```

From here we add methods that will test different "units" of our code.
This is done through a range of helper methods provided (see table of contents) 
to us by the `UnitTest` class.

```cs
from UnitTest import UnitTest;

def add(a, b) {
    return a + b;
}

class Test < UnitTest {
    testAddFunction() {
        this.assertEquals(add(2, 3), 5);
    }
}

Test().run();
```

The framework will search any methods within the class and execute any that 
begin with `test`, if a method within a class does not begin with `test` it is simply
ignored. The final step is to ensure we run the unit test by calling the `.run()` method.

This will generate the following output.

```
file.du
    testAddFunction()
         Success.

Results:
    - 1 assertion(s) were successful.
    - 0 assertion(s) were failures.
    - 0 method(s) were skipped.
```

## Helpers

Along with the list of assertion methods in the next section, there are a few
helper methods to make test writing easier.

### setUp

`setUp` is a method which is ran before a test method is executed. This is useful
if you need some data (for example) to be present before running a test.

Note: If in use with providers `setUp` will run for every item passed into the array.

```cs
class Test < UnitTest {
    setUp() {
        // Code
    }
}
```

### tearDown

`tearDown` very similar to `setUp` yet happens once a test has completed. Useful
for cleaning up any data created in `setUp` or the test itself (examples).

Note: If in use with providers `tearDown` will run for every item passed into the array.

```cs
class Test < UnitTest {
    tearDown() {
        // Code
    }
}
```

### Providers

Providers are special methods that allow you to pass data into tests 
so that they can be made slightly more dynamic and reduce the boiler plate
needing to be written. They are denoted by appending a method name with `Provider`.

Providers can pass a single value and you can use this within your test, or if
you return an array from a provider then the test will run for every item within the
array.

```cs
from UnitTest import UnitTest;

def add(a, b) {
    return a + b;
}

class Test < UnitTest {
    testAddFunction(data) {
        this.assertEquals(add(data["val1"], data["val2"]), data["expected"]);
    }

    // This will not be ran as a test as it's marked as a provider
    testAddFunctionProvider() {
        return [
            {"val1": 1, "val2": 2, "expected": 3},
            {"val1": 2, "val2": 2, "expected": 4},
            {"val1": 3, "val2": 2, "expected": 5},
            {"val1": 4, "val2": 2, "expected": 6},
            {"val1": 5, "val2": 2, "expected": 7},
            {"val1": 6, "val2": 2, "expected": 8},
        ];
    }
}

Test().run();
```

As you can see, providers make it incredibly easy to add additional test cases without 
the need to actually modify your test code.

Output:
```
file.du
    testAddFunction()
         Success.
         Success.
         Success.
         Success.
         Success.
         Success.

Results:
    - 6 assertion(s) were successful.
    - 0 assertion(s) were failures.
    - 0 method(s) were skipped.
```

### Skipping Tests

Sometimes we need to skip tests, be that because they're temporarily broken or
because they're testing functionality that has been disabled for whatever reason.

This is done very simply by just appending `_skipped` to the method name.


```cs
from UnitTest import UnitTest;

def add(a, b) {
    return a + b;
}

class Test < UnitTest {
    testAddFunction_skipped() {
        this.assertEquals(add(2, 3), 5);
    }
}

Test().run();
```

Output:
```
file.du

Results:
    - 0 assertion(s) were successful.
    - 0 assertion(s) were failures.
    - 1 method(s) were skipped.
```

## Settings
### Silencing Passing Tests
The default setting is that it will output `Success.` for tests that pass, but
sometimes that can cause finding errors or tests that fail slightly harder 
so we can turn this off to only show output from failed tests.

```cs
from UnitTest import UnitTest;

def add(a, b) {
    return a + b;
}

class Test < UnitTest {
    init() {
        // Remember to call the parent constructor!
        super.init();

        // Set it so only errors display
        this.onlyFailures = true;
    }

    testAddFunction(data) {
        this.assertEquals(add(data["val1"], data["val2"]), data["expected"]);
    }

    // This will not be ran as a test as it's marked as a provider
    testAddFunctionProvider() {
        return [
            {"val1": 1, "val2": 2, "expected": 3},
            {"val1": 2, "val2": 2, "expected": 4},
            {"val1": 3, "val2": 2, "expected": 5},
            {"val1": 4, "val2": 2, "expected": 6},
            {"val1": 5, "val2": 2, "expected": 7},
            // This will fail
            {"val1": 6, "val2": 2, "expected": 9},
        ];
    }
}

Test().run();
```

Output:
```
file.du
    testAddFunction()
         Line: 17 - Failure: 8 is not equal to 9.

Results:
    - 5 assertion(s) were successful.
    - 1 assertion(s) were failures.
    - 0 method(s) were skipped.
```

### Exit On Failure
Sometimes we may want our test suite to stop as soon as a test fails.
This is done very similarly to silencing passing tests.

```cs
from UnitTest import UnitTest;

def add(a, b) {
    return a + b;
}

class Test < UnitTest {
    init() {
        // Remember to call the parent constructor!
        super.init();

        // Exit as soon as we get a failure
        this.exitOnFailure = true;
    }

    testAddFunction(data) {
        this.assertEquals(add(data["val1"], data["val2"]), data["expected"]);
    }

    // This will not be ran as a test as it's marked as a provider
    testAddFunctionProvider() {
        return [
            // This will fail
            {"val1": 1, "val2": 2, "expected": 5},
            {"val1": 2, "val2": 2, "expected": 4},
            {"val1": 3, "val2": 2, "expected": 5},
            {"val1": 4, "val2": 2, "expected": 6},
            {"val1": 5, "val2": 2, "expected": 7},
            // This will fail
            {"val1": 6, "val2": 2, "expected": 9},
        ];
    }
}

Test().run();
```

In this case the first dictionary in the data provider will cause our add test
to fail, with `exitOnFailure` set to true it will exit after the first assertion.

Output:
```
file.du
    testAddFunction()
         Line: 17 - Failure: 3 is not equal to 5.
```

## Assertions
### assertEquals(value, value)

This helper method ensures that both values passed in equal each other.

### assertTruthy(value)

This helper method ensures that the value passed in would evaluate to true.

Note: This is not the same as equaling `true`.

### assertFalsey(value)

This helper method ensures that the value passed in would evaluate to false.

Note: This is not the same as equaling `false`.

### assertSuccess(value)

This helper method ensures that the value passed in is a `Result` type in
a `Success` state. 

### assertError(value)

This helper method ensures that the value passed in is a `Result` type in
an `Error` state. 

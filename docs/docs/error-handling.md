---
layout: default
title: Error Handling
nav_order: 14
---

# Error Handling
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

## Error Handling

Dictu does not have exceptions like many other languages, and instead
uses a Result type. A Result is a type which can be in one of two states,
SUCCESS or ERROR. Logic which may return an error will always return a Result
type which will wrap a value on success or wrap a string on failure with a given
error message. This wrapped value *must* be unwrapped before accessing it.

Because Dictu has no exceptions, it also has no way to catch a runtime error.
Unwrapping a Result that is in an ERROR state raises a runtime error, which
terminates the program. **Always check a Result before unwrapping it** - the
idiomatic ways to do that are described below.

### Result type

Note, if returning a Result type from a function there is nothing in the interpreter
that will enforce both Success and Error types can be returned, or even that these are the only
types that can be returned, however it is very much recommended that if you return a Result type
from a function, this is the only type you ever return - this will make handling a result type
much easier for the caller.

#### Success

Creating a Success type is incredibly simple with the builtin `Success()` function.
Any type can be passed to Success to be wrapped.

```cs
var result = Success(10);
```

#### Error

Creating an Error type is incredibly simple with the builtin `Error()` function.
Only a string can be passed to Error to be wrapped.

```cs
var result = Error("Some error happened!!");
```

## Checking a Result

### Truthiness

A Result in an ERROR state is falsey, and a Result in a SUCCESS state is truthy.
This means a Result can be checked directly in a conditional, without calling any
method on it.

```cs
const result = "10".toNumber();

if (not result) {
    print("Could not parse the number");
}
```

This checks the *state* of the Result, not the value it wraps, so a Success that
wraps a falsey value is still truthy.

```cs
if (Success(0)) {
    print("Still truthy - 0 is the wrapped value, not the state");
}
```

See the [truthy / falsey](/docs/variables#truthy--falsey) section for the full list of
falsey values.

### .success() -> Boolean

`.success()` is the explicit form of the same check, and returns a boolean.

```cs
"10".toNumber().success(); // true
"number".toNumber().success(); // false
```

Use whichever reads better. `if (not result)` is more concise, `.success()` is more
explicit about what is being tested.

## Propagating errors

Because a Result is just a value, an error is propagated by returning it. Guard each
call that can fail, and return the Result unchanged if it failed. Once past the guard,
`.unwrap()` cannot fail.

```cs
def parsePoint(input) {
    const parts = input.split(",");

    const x = parts[0].toNumber();
    if (not x) return x;

    const y = parts[1].toNumber();
    if (not y) return y;

    return Success([x.unwrap(), y.unwrap()]);
}
```

The caller then checks once, and only the outermost caller needs to decide what to
do about the failure.

```cs
const point = parsePoint("10,20");
if (not point) {
    print("Could not parse the point: {}".format(point.unwrapError()));
    System.exit(1);
}

print(point.unwrap()); // [10, 20]
```

To add context to an error as it travels up, wrap the message in a new Error.

```cs
const x = parts[0].toNumber();
if (not x) {
    return Error("parsing x coordinate: {}".format(x.unwrapError()));
}
```

## Unwrapping

### .unwrap() -> Value

`.unwrap()` returns the value wrapped by a Result in a SUCCESS state.

Attempting to unwrap a Result that is in an ERROR state raises a runtime error which
cannot be caught, so only call `.unwrap()` once you have established the Result is a
success.

```cs
const num = "10".toNumber();
print(num); // <Result Suc>

if (num) {
    print(num.unwrap()); // 10
}
```

### .unwrapError() -> String

A Result that has a type of ERROR will always contain an error message as to why it failed, however 
attempting to unwrap a Result that is an ERROR gives you a runtime error. Instead you must use
`.unwrapError()`. Attempting to use `unwrapError` on a Result with type SUCCESS will raise a runtime
error.

```cs
"num".toNumber().unwrapError(); // "Cannot convert 'num' to number"
```

## Matching

The match methods take callbacks and are a convenient way to handle both states of a
Result in a single expression.

Note that these callbacks cannot be used to propagate an error out of the enclosing
function. A `return` inside a callback returns from the callback itself, not from the
function containing the `.match()` call, so execution continues afterwards. Use the
callbacks to handle an error where it occurs - by logging it, exiting, or substituting
a value - and use the guard pattern described in [propagating errors](#propagating-errors)
when the error needs to travel up to the caller.

### .match(Func: success, Func: error) -> Value

`.match` takes two callbacks that are ran depending upon the status of the result type. The callbacks passed to
match must both have one parameter each, on success the unwrapped value is passed as the first argument and on
error the unwrapError reason is passed to the failure callback. The value returned from `.match()` is the value
returned from the user defined callback.

```cs
var number = "10".toNumber().match(
    def (result) => result,
    def (error) => {
        print(error);
        System.exit(1);
    }
);

print(number); // 10

var number = "number".toNumber().match(
    def (result) => result,
    def (error) => {
        print(error); // Can not convert 'number' to number
        System.exit(1);
    }
);

print(number);
```

### .matchWrap(Func: success, Func: error) -> Result

`.matchWrap` is exactly the same as `.match` however, the value returned from either callback
function is implicitly wrapped back up into a Result object. This allows us to easily deal
with the error at a different call site and avoids the necessity for explicit wrapping.

```cs
var response = HTTP.get("https://some/endpoint").matchWrap(
    def (data) => JSON.parse(data).unwrap(),
    def (error) => error
);

print(response); // <Result Suc>
```

In the above example we can handle the case that we need to do some data transformation, however, we
also need to ensure that a Result object is returned in case we hit the error callback.

### .matchError(Func: error) -> Value

`.matchError` is similar to `.match` except it only accepts a single callback. If the Result type is 
in an Error state the callback will be ran, otherwise the Result will be unwrapped and the wrapped value is
returned. It becomes a shortcut if you do not need any intermediate handling on success unwrapping. 

```cs
var number = "10".toNumber().matchError(
    def (error) => {
        print(error);
        System.exit(1);
    }
);

print(number); // 10
```

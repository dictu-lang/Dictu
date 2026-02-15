---
layout: default
title: Fiber
nav_order: 8
parent: Standard Library
---

# Fiber
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Fiber

Fibers are lightweight cooperative coroutines. Each fiber has its own call stack, and execution can be suspended and resumed at will. They are useful for generators, pipelines, and cooperative multitasking.

To make use of the Fiber module an import is required.

```js
import Fiber;
```

### How Fibers Work

A fiber wraps a function and gives it the ability to pause itself in the middle of execution. When paused, control returns to the code that was running the fiber. The next time the fiber is called, it picks up right where it left off.

```cs
var f = Fiber.new(def () => {
    print("before yield");
    Fiber.yield();
    print("after yield");
});

f.call();   // "before yield"
f.call();   // "after yield"
```

### Passing Values

Fibers support bidirectional value passing. The value passed to `call()` is delivered to the fiber, and the value passed to `yield()` is returned from `call()`.

On the very first `call()`, the value is delivered as the function parameter. On subsequent calls, it becomes the return value of `Fiber.yield()` inside the fiber.

```cs
var f = Fiber.new(def (first) => {
    print(first);                     // 10
    var second = Fiber.yield(first * 2);
    print(second);                    // 100
    return second * 3;
});

print(f.call(10));   // 20  (yielded value: 10 * 2)
print(f.call(100));  // 300 (returned value: 100 * 3)
```

### Generators

A common use of fibers is to create lazy generators that produce values on demand.

```cs
var fib = Fiber.new(def () => {
    var a = 0;
    var b = 1;
    while (true) {
        Fiber.yield(a);
        var temp = a + b;
        a = b;
        b = temp;
    }
});

for (var i = 0; i < 8; i += 1) {
    print(fib.call());
}
// 0, 1, 1, 2, 3, 5, 8, 13
```

### Fiber.new(Function) -> Fiber

Creates a new fiber from the given function. The function can take zero or one parameter. If it takes one parameter, the value passed to the first `call()` is delivered as that parameter.

```cs
var f = Fiber.new(def () => {
    return 42;
});

print(f); // <Fiber>
```

### Fiber.yield(Value: value -> Optional) -> Value

Suspends the current fiber and returns control to its caller. The optional value is sent back to the caller as the return value of `call()`. When the fiber is later resumed, `yield()` returns the value passed to the next `call()`.

Can only be called from within a non-main fiber.

```cs
var f = Fiber.new(def () => {
    Fiber.yield("hello");
    return "done";
});

print(f.call());  // "hello"
print(f.call());  // "done"
```

### Fiber.isMain() -> Bool

Returns true if the currently executing code is running on the main fiber.

```cs
print(Fiber.isMain()); // true

var f = Fiber.new(def () => {
    print(Fiber.isMain()); // false
    return nil;
});

f.call();
```

### fiber.call(Value: value -> Optional) -> Value

Resumes the fiber, optionally passing a value. Returns the value yielded by `Fiber.yield()` or the return value of the fiber's function when it completes.

It is a runtime error to call a fiber that has already finished.

```cs
var f = Fiber.new(def () => {
    Fiber.yield(1);
    Fiber.yield(2);
    return 3;
});

print(f.call()); // 1
print(f.call()); // 2
print(f.call()); // 3
// f.call() would be an error here
```

### fiber.isDone() -> Bool

Returns true if the fiber has finished execution and can no longer be called.

```cs
var f = Fiber.new(def () => {
    return 42;
});

print(f.isDone()); // false
f.call();
print(f.isDone()); // true
```

### fiber.toString() -> String

Returns a string representation of the fiber.

```cs
var f = Fiber.new(def () => { return nil; });
print(f.toString()); // "<Fiber>"
```

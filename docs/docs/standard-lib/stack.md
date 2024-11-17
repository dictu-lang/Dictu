---
layout: default
title: Stack
nav_order: 23
parent: Standard Library
---

# UUID
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Stack

To make use of the Stack module an import is required.

```js
import Stack;
```

### Stack.new() -> Stack

Returns a Stack value with the default size of 8.

```cs
const stack = Stack.new();
print(stack);
// <Stack>
```

### Stack.newWithSize(Number) -> Result\<Stack>

Returns a Result value with a Stack initialized to the given size. An error Result is returned if the given capacity is less than 0.

```cs
const stack = Stack.newWithSize(10).unwrap();
print(stack);
// <Stack>
```

### Stack.push(Value) -> Nil

Adds the given value to the stack and resizes the stack if necessary.

```cs
const stack = Stack.new();
stack.push("some_value");
```

### Stack.pop() -> Value

Returns the value at the top of the stack. The capacity will be decreased if necessary.

**Note:** A runtime error will occur if there is an attempt to pop on an empty stack

```cs
const stack = Stack.new();
stack.push("some_value");
const value = stack.pop();
print(value);
// some_value
```

### Stack.peek() -> Value

Returns the value at the top of the stack without removing the value.

```cs
const stack = Stack.new();
stack.push("some_value");
const value = stack.peek();
print(value);
// some_value
```

### Stack.cap() -> Number

Returns a Number value of the capacity of the stack. The capacity is the overall size of the stack.

```cs
const stack = Stack.new();
const cap = stack.cap();
print(cap);
// 8
```

### Stack.len() -> Number

Returns a Number value of the length of the stack. The length represents the number of items in the stack.

```cs
const stack = Stack.new();
const len = stack.len();
print(len);
// 0
```

### Stack.isEmpty() -> Bool

Returns a Bool value indicating whether the stack is empty or not.

```cs
const stack = Stack.new();
const res = stack.isEmpty();
print(res);
// true
```

### Stack.isFull() -> Bool

Returns a Bool value indicating whether the stack is empty or not.

```cs
const stack = Stack.new();
const res = stack.isFull();
print(res);
// false
```

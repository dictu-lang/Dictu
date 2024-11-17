---
layout: default
title: Queue
nav_order: 19
parent: Standard Library
---

# UUID
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Queue

To make use of the Queue module an import is required.

```js
import Queue;
```

### Queue.new() -> Queue

Returns a Queue value with the default size of 8.

```cs
const queue = Queue.new();
print(queue);
// <Queue>
```

### Queue.newWithSize(Number) -> Result\<Queue>

Returns a Result value with a Queue initialized to the given size. An error Result is returned if the given capacity is less than 0.

```cs
const queue = Queue.newWithSize(10).unwrap();
print(queue);
// <Queue>
```

### Queue.push(Value) -> Nil

Adds the given value to the queue and resizes the queue if necessary.

```cs
const queue = Queue.new();
queue.push("some_value");
```

### Queue.pop() -> Value

Returns the value at the front of the queue. The capacity will be decreased if necessary.

```cs
const queue = Queue.new();
queue.push("some_value");
const value = queue.pop();
print(value);
// some_value
```

### Queue.peek() -> Value

Returns the value at the front of the queue without removing the value.

```cs
const queue = Queue.new();
queue.push("some_value");
const value = queue.peek();
print(value);
// some_value
```

### Queue.cap() -> Number

Returns a Number value of the capacity of the queue. The capacity is the overall size of the queue.

```cs
const queue = Queue.new();
const cap = queue.cap();
print(cap);
// 8
```

### Queue.len() -> Number

Returns a Number value of the length of the queue. The length represents the number of items in the queue.

```cs
const queue = Queue.new();
const len = queue.len();
print(len);
// 0
```

### Queue.isEmpty() -> Bool

Returns a Bool value indicating whether the queue is empty or not.

```cs
const queue = Queue.new();
const res = queue.isEmpty();
print(res);
// true
```

### Queue.isFull() -> Bool

Returns a Bool value indicating whether the queue is empty or not.

```cs
const queue = Queue.new();
const res = queue.isFull();
print(res);
// false
```

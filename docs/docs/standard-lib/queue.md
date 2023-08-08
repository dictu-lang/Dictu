---
layout: default
title: Queue
nav_order: 20
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

```cs
import Queue;
```

### Queue.new() -> Result<Queue>

Returns a Result value with a Queue with the default size of 8 or an error.

```cs
const queue = Queue.new().unwrap();
print(queue);
// <Queue>
```

### Queue.newWithSize(Number) -> Result<Queue>

Returns a Result value with a Queue initialized to the given size.

```cs
const queue = Queue.newWithSize(10).unwrap();
print(queue);
// <Queue>
```

### Queue.push(Any) -> Result<Queue>

Adds the given argument to the queue and resizes the queue if necessary.

```cs
const queue = Queue.new().unwrap();
queue.push("some_value");
```

### Queue.pop() -> Result<Queue>

Returns a Result value with the expected value on success or `Nil` on faiure. The capacity will be decreased as necessary.

```cs
const queue = Queue.new().unwrap();
queue.push("some_value");
const value = queue.pop();
print(value);
// some_value
```

### Queue.cap() -> Number

Returns a Number value of the capacity of the queue. The capacity is the overall size of the queue.

```cs
const queue = Queue.new().unwrap();
const cap = queue.cap();
print(cap);
// 8
```

### Queue.len() -> Number

Returns a Number value of the length of the queue. The length represents the number of items in the queue.

```cs
const queue = Queue.new().unwrap();
const len = queue.len();
print(len);
// 0
```

### Queue.isEmpty() -> Bool

Returns a Bool value indicating whether the queue is empty or not.

```cs
const queue = Queue.new().unwrap();
const res = queue.isEmpty();
print(res);
// true
```

### Queue.isFull() -> Bool

Returns a Bool value indicating whether the queue is empty or not.

```cs
const queue = Queue.new().unwrap();
const res = queue.isFull();
print(res);
// false
```

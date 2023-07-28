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

Returns a Result value with a Queue or an error. `Queue.new()` takes 1 argument of type Number for the size of the queue.

```cs
const queue = Queue.new(10).unwrap();
print(bi);
// <Queue>
```

### Queue.push() -> Result<Queue>

Returns a Result value with a `Nil` on success. The capacity will be increased as necessary.

```cs
const queue = Queue.new(10).unwrap();
queue.push("some_value");
```

### Queue.pop() -> Result<Queue>

Returns a Result value with the expected value on success or `Nil` on faiure. The capacity will be decreased as necessary.

```cs
const queue = Queue.new(10).unwrap();
queue.push("some_value");
const value = queue.pop();
print(value);
// some_value
```

### Queue.cap() -> Number

Returns a Number value of the capacity of the queue. The capacity is the overall size of the queue.

```cs
const queue = Queue.new(10).unwrap();
const cap = queue.cap();
print(cap);
// 10
```

### Queue.len() -> Number

Returns a Number value of the length of the queue. The length represents the number of items in the queue.

```cs
const queue = Queue.new(10).unwrap();
const len = queue.len();
print(len);
// 0
```

### Queue.isEmpty() -> Bool

Returns a Bool value indicating whether the queue is empty or not.

```cs
const queue = Queue.new(10).unwrap();
const res = queue.isEmpty();
print(res);
// true
```

### Queue.isFull() -> Bool

Returns a Bool value indicating whether the queue is empty or not.

```cs
const queue = Queue.new(10).unwrap();
const res = queue.isFull();
print(res);
// false
```

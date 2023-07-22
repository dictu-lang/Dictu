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

Returns a Result value with a `Nil` on success.

```cs
const queue = Queue.new(10).unwrap();
queue.push("some_value");
```

### Queue.pop() -> Result<Queue>

Returns a Result value with the expected value on success or `Nil` on faiure.

```cs
const queue = Queue.new(10).unwrap();
queue.push("some_value");
const value = queue.pop();
print(value);
// some_value
```

### Queue.size() -> Number

Returns a Number value of the size of the queue.

```cs
const queue = Queue.new(10).unwrap();
const size = queue.size();
print(size);
// 10
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

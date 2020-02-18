---
layout: default
title: Math
nav_order: 11
---

# Math
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Math

Functions relating to mathematics are behind the Math namespace. For the purpose of the documentation, and iterable
is either a list, or passing multiple arguments to the function directly.

### Math.min(iterable)

Return the smallest number within the iterable

```js
Math.min(1, 2, 3); // 1
Math.min([1, 2, 3]); // 1
```

### Math.max(iterable)

Return the largest number within the iterable

```js
Math.min(1, 2, 3); // 3
Math.min([1, 2, 3]); // 3
```

### Math.average(iterable)

Return the average of the iterable

```js
Math.average(1, 2, 3); // 2
Math.average([1, 2, 3]); // 2
```

### Math.sum(iterable)

Return the sum of the iterable

```js
Math.sum(1, 2, 3); // 6
Math.sum([1, 2, 3]); // 6
```

### Math.floor(number)

Return the largest integer less than or equal to the given input

```js
Math.floor(17.4); // 17
Math.floor(17.999); // 17
Math.floor(17); // 17
```

### Math.round(number)

Round to the nearest integer

```js
Math.round(17.4); // 17
Math.round(17.49); // 17
Math.round(17.5); // 18
```

### Math.ceil(number)

Returns smallest integer greater than or equal to given input

```js
Math.ceil(17.4); // 18
Math.ceil(17.999); // 18
Math.ceil(17); // 17
```

### Math.abs(number)

Returns absolute value of a given number

```js
Math.abs(-10); // 10
Math.abs(10); // 10
Math.abs(-10.5); // 10.5
```

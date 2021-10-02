---
layout: default
title: Math
nav_order: 3
parent: Standard Library
---

# Math
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Math

To make use of the Math module an import is required. For the purpose of the documentation, an iterable 
is either a list, or passing multiple arguments to the function directly.

```cs
import Math;
```

### Constants

| Constant     | Description                                            |
|--------------|--------------------------------------------------------|
| Math.PI      | The mathematical constant: 3.14159265358979            |
| Math.e       | The mathematical constant: 2.71828182845905            |

### Math.min(iterable)

Return the smallest number within the iterable

```cs
Math.min(1, 2, 3); // 1
Math.min([1, 2, 3]); // 1
```

### Math.max(iterable)

Return the largest number within the iterable

```cs
Math.max(1, 2, 3); // 3
Math.max([1, 2, 3]); // 3
```

### Math.average(iterable)

Return the average of the iterable

```cs
Math.average(1, 2, 3); // 2
Math.average([1, 2, 3]); // 2
```

### Math.sum(iterable)

Return the sum of the iterable

```cs
Math.sum(1, 2, 3); // 6
Math.sum([1, 2, 3]); // 6
```

### Math.floor(number)

Return the largest integer less than or equal to the given input

```cs
Math.floor(17.4); // 17
Math.floor(17.999); // 17
Math.floor(17); // 17
```

### Math.round(number)

Round to the nearest integer

```cs
Math.round(17.4); // 17
Math.round(17.49); // 17
Math.round(17.5); // 18
```

### Math.ceil(number)

Returns smallest integer greater than or equal to given input

```cs
Math.ceil(17.4); // 18
Math.ceil(17.999); // 18
Math.ceil(17); // 17
```

### Math.abs(number)

Returns absolute value of a given number

```cs
Math.abs(-10); // 10
Math.abs(10); // 10
Math.abs(-10.5); // 10.5
```

### Math.sqrt(number)

Returns the square root of a given number

```cs
Math.sqrt(25); // 5
Math.sqrt(100); // 10
```

### Math.sin(number)

Returns the sin value of a given number in radian

```cs
Math.sin(1); // 0.8414
Math.sin(50); // -0.2623
```

### Math.cos(number)

Returns the cos value of a given number in radian

```cs
Math.cos(1); // 0.5403
Math.cos(50); // 0.9649
```

### Math.tan(number)

Returns the tan value of a given number in radian

```cs
Math.tan(1); // 1.5574
Math.tan(50); // -0.2719
```

### Math.gcd(iterable)

Return the greatest common divisor of the numbers within the iterable

```cs
Math.gcd(32, 24, 12); // 4
Math.gcd([32, 24, 12]); // 4
```

### Math.lcm(iterable)

Return the least common multiple of the numbers within the iterable

```cs
Math.lcm(32, 24, 12); // 96
Math.lcm([32, 24, 12]); // 96
```

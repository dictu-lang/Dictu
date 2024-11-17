---
layout: default
title: Math
nav_order: 14
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

```js
import Math;
```

### Constants

| Constant     | Description                                 |
| ------------ | ------------------------------------------- |
| Math.pi      | The mathematical constant: 3.14159265358979 |
| Math.e       | The mathematical constant: 2.71828182845905 |
| Math.phi     | The mathematical constant: 1.61803398874989 |
| Math.sqrt2   | The mathematical constant: 1.41421356237309 |
| Math.sqrte   | The mathematical constant: 1.61803398874989 |
| Math.sqrtpi  | The mathematical constant: 1.77245385090551 |
| Math.sqrtphi | The mathematical constant: 1.27201964951406 |
| Math.ln2     | The mathematical constant: 0.69314718055994 |
| Math.ln10    | The mathematical constant: 2.30258509299404 |

### Math.min(...Number|List) -> Number

Return the smallest number within the iterable.

```cs
Math.min(1, 2, 3); // 1
Math.min([1, 2, 3]); // 1
```

### Math.max(...Number|List) -> Number

Return the largest number within the iterable.

```cs
Math.max(1, 2, 3); // 3
Math.max([1, 2, 3]); // 3
```

### Math.average(...Number|List) -> Number

Return the average of the iterable.

```cs
Math.average(1, 2, 3); // 2
Math.average([1, 2, 3]); // 2
```

### Math.sum(...Number|List) -> Number

Return the sum of the iterable.

```cs
Math.sum(1, 2, 3); // 6
Math.sum([1, 2, 3]); // 6
```

### Math.floor(Number) -> Number

Return the largest integer less than or equal to the given input.

```cs
Math.floor(17.4); // 17
Math.floor(17.999); // 17
Math.floor(17); // 17
```

### Math.round(Number) -> Number

Round to the nearest integer.

```cs
Math.round(17.4); // 17
Math.round(17.49); // 17
Math.round(17.5); // 18
```

### Math.ceil(Number) -> Number

Returns smallest integer greater than or equal to given input.

```cs
Math.ceil(17.4); // 18
Math.ceil(17.999); // 18
Math.ceil(17); // 17
```

### Math.abs(Number) -> Number

Returns absolute value of a given number.

```cs
Math.abs(-10); // 10
Math.abs(10); // 10
Math.abs(-10.5); // 10.5
```

### Math.sqrt(Number) -> Number

Returns the square root of a given number.

```cs
Math.sqrt(25); // 5
Math.sqrt(100); // 10
```

### Math.sin(Number) -> Number

Returns the sin value of a given number in radian.

```cs
Math.sin(1); // 0.8414
Math.sin(50); // -0.2623
```

### Math.cos(Number) -> Number

Returns the cos value of a given number in radian.

```cs
Math.cos(1); // 0.5403
Math.cos(50); // 0.9649
```

### Math.tan(Number) -> Number

Returns the tan value of a given number in radian.

```cs
Math.tan(1); // 1.5574
Math.tan(50); // -0.2719
```

### Math.gcd(...Number|List) -> Number

Return the greatest common divisor of the numbers within the iterable.

```cs
Math.gcd(32, 24, 12); // 4
Math.gcd([32, 24, 12]); // 4
```

### Math.lcm(...Number|List) -> Number

Return the least common multiple of the numbers within the iterable.

```cs
Math.lcm(32, 24, 12); // 96
Math.lcm([32, 24, 12]); // 96
```

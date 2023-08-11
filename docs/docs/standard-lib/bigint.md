---
layout: default
title: BigInt
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

## BigInt

To make use of the UUID module an import is required.

```cs
import BigInt;
```

### BigInt.new() -> Result<BigInt>

Returns a Result value with a BigInt or an error. `BigInt.new()` takes 0 or 1 argument of either Number or String. If no argument is given, the returned BigInt value is set to 0.

```cs
const bi = BigInt.new().unwrap();
print(bi);
// 0
```

```cs
const bi = BigInt.new(8675309).unwrap();
print(bi);
// 8675309
```

```cs
const bi = BigInt.new("678").unwrap();
print(bi);
// 678
```

### bigInt1.compare(bigInt2) -> Number

Returns a Number value of either 0 indicating the 2 BigInts compared are the same and -1 if the values are different.

```cs
const bi1 = BigInt.new(10).unwrap();
const bi2 = BigInt.new(10).unwrap();
const value = bi1.compare(bi2);
print(value);
// 0
```

### bigIntVal.negate() -> Result<BigInt>

Returns a Result value that unwraps to the negated BigInt value.

```cs
const bi = BigInt.new(10).unwrap();

const value = bi1.negate();
print(value);
// -10
```

### bigIntVal.abs() -> Result<BigInt>

Returns a Result value whose value is the absolute value of the BigInt.

```cs
const bi = BigInt.new(10).unwrap();

const value = bi1.abs();
print(value);
// 10
```

### bigIntVal.bitwiseAnd() -> Result<BigInt>

Returns a Result value whose value is the absolute value of the BigInt.

```cs
const bi = BigInt.new(10).unwrap();

const value = bi1.abs();
print(value);
// 10
```

---
layout: default
title: BigInt
nav_order: 3
parent: Standard Library
---

# BigInt
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## BigInt

To make use of the BigInt module an import is required.

```js
import BigInt;
```

### BigInt.new() -> Result\<BigInt>

Returns a Result value with a BigInt or an error. Takes 0 or 1 argument of either Number or String. If no argument is given, the returned BigInt value is set to 0.

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

### bigIntVal.negate() -> Result\<BigInt>

Returns a Result value that unwraps to the negated BigInt value.

```cs
const bi = BigInt.new(10).unwrap();

const value = bi1.negate();
print(value);
// -10
```

### bigIntVal.abs() -> Result\<BigInt>

Returns a Result containing the absolute value of the BigInt.

```cs
const bi = BigInt.new(10).unwrap();

const value = bi1.abs();
print(value);
// 10
```

### bigIntVal1.bitwiseAnd() -> Result\<BigInt>

Returns a Result containing the value of `bigIntVal1 & bigIntVal2`.

```cs
const bi1 = BigInt.new(102216).unwrap();
const bi2 = BigInt.new(8909).unwrap();

const value = bi1.bitwiseAnd(bi2);
print(value);
// 584
```

### bigIntVal1.bitwiseOr() -> Result\<BigInt>

Returns a Result containing the value of `bigIntVal1 | bigIntVal2`.

```cs
const bi1 = BigInt.new(102216).unwrap();
const bi2 = BigInt.new(8909).unwrap();

const value = bi1.bitwiseOr(bi2);
print(value);
// 110541
```

### bigIntVal1.bitwiseXor() -> Result\<BigInt>

Returns a Result containing the value of `bigIntVal1 ^ bigIntVal2`.

```cs
const bi1 = BigInt.new(102216).unwrap();
const bi2 = BigInt.new(8909).unwrap();

const value = bi1.bitwiseXor(bi2);
print(value);
// 109957
```

### bigIntVal.add() -> Result\<BigInt>

Returns a Result value containing the sum.

```cs
const bi = BigInt.new(10).unwrap();

const value = bi1.add();
print(value);
// 10
```

### bigIntVal.subtract() -> Result\<BigInt>

Returns a Result value containing the difference.

```cs
const bi = BigInt.new(10).unwrap();

const value = bi1.subtract();
print(value);
// 10
```

### bigIntVal.multiply() -> Result\<BigInt>

Returns a Result value containing the product.

```cs
const bi = BigInt.new(10).unwrap();

const value = bi1.multiply();
print(value);
// 10
```

### bigIntVal.divide() -> Result\<BigInt>

Returns a Result value containing the quotiant.

```cs
const bi = BigInt.new(10).unwrap();

const value = bi1.divide();
print(value);
// 10
```

### bigIntVal.modulo() -> Result\<BigInt>

Returns a Result value containing the remainder.

```cs
const bi = BigInt.new(10).unwrap();

const value = bi1.modulo();
print(value);
// 10
```

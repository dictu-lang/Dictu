---
layout: default
title: Operators
nav_order: 6
---

# Operators
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## Operators

| Operator     | Description                                                    | Example                    |
|:-------------|:---------------------------------------------------------------|:---------------------------|
| and          | Returns true if both operands are truthy values otherwise false | true and true |
| or           | Returns the first truthy operand unless all are false and the last operand is returned | true or false |
| not          | Returns the inverse boolean value for a given operand | not true |
| +            | Adds the values on either side of the operator together        | 10 + 10                    |
| -            | Subtracts the values on either side of the operator together   | 10 - 10                    |
| *            | Multiplies the values on either side of the operator together  | 10 * 2                     |
| /            | Divides the values on either side of the operator together.    | 10 / 3                     |
| <            | Returns true if the left operand is less than the right operand (for strings this is based on the ASCII value)    | 9 < 10 |
| >            | Returns true if the left operand is more than the right operand (for strings this is based on the ASCII value)    | 11 > 10 |
| <=           | Same as < however includes an equality check                   | 9 <= 9                     |
| >=           | Same as > however includes an equality check                   | 10 >= 10                   |
| %            | Modulo of values on either side of the operator                | 10 % 2                     |
| **           | Exponent (power) of the values                                 | 2 ** 2                     |
| &            | Bitwise AND of the values                                      | 10 & 2                     |
| ^            | Bitwise XOR of the values                                      | 10 ^ 2                     |
| \|            | Bitwise OR of the values                                       | 10 \| 2                     |
| +=           | Same as +, however its shorthand to assign too                 | x += 10 Same as x = x + 10 |
| -=           | Same as -, however its shorthand to assign too                 | x -= 10 Same as x = x - 10 |
| *=           | Same as *, however its shorthand to assign too                 | x *= 10 Same as x = x * 10 |
| /=           | Same as /, however its shorthand to assign too                 | x /= 10 Same as x = x / 10 |
| &=           | Same as &, however its shorthand to assign too                 | x &= 10 Same as x = x & 10 |
| ^=           | Same as ^, however its shorthand to assign too                 | x ^= 10 Same as x = x ^ 10 |
| \|=           | Same as \|, however its shorthand to assign too                 | x \|= 10 Same as x = x |
| ?            | Ternary operator - See below                                   | true ? 'value' : 'other' |
| ?.           | Optional chaining - See [classes](/docs/classes/#optional-chaining) | object?.someMethod()     |

### Ternary Operator

The ternary operator is an operator which takes 3 operands and returns either the second or third depending on whether the first operand is truthy. 

```cs
var value = true ? 'true!' : 'false!';
print(value); // 'true!'

var otherValue = 0 ? 'true!' : 'false!';
print(otherValue); // 'false!'
```

## Precedence

Precedence table from highest to lowest, with all operators having a left-to-right associativity. 

| Operators |
| . () []   |
| ?.        |
| ! -       |
| \*\*      |
| * /       |
| \+ \-     |
| &         |
| ^         |
| \|        |
| < > <= >= |
| == !=     |
| and       |
| or        |
| \=        |
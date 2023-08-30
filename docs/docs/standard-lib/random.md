---
layout: default
title: Random
nav_order: 16
parent: Standard Library
---

# Random
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Random
To make use of the Random module an import is required.

**Note:** This is not cryptographically secure and should not be used for such purposes.

```js
import Random;
```

### Random.random() -> Number

Return a random decimal between 0 and 1.

```cs
Random.random(); // 0.314
Random.random(); // 0.271
```

### Random.range(Number: lowest, Number: highest) -> Number

Returns a random integer between the lowest and highest inputs where both are inclusive.

```cs
Random.range(1, 5); // 2
Random.range(1, 5); // 4
Random.range(0, 2); // 1
```

### Random.select(List) -> value

Returns a value randomly selected from the list.

```js
Random.select([2, 4, 6]);  // 6
Random.select([2, 4, 6]);  // 2
Random.select(["a", "b", "c"]); // "c"
```

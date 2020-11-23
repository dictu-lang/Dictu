---
layout: default
title: Base64
nav_order: 21
---

# Random
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Base64
To make use of the Base64 module an import is required.

```js
import Base64;
```

### Constants

| Constant             | Description                     |
|----------------------|---------------------------------|
| Base64.errno         | Number of the last error        |

### Base64.encode(string)

Base64 encode a given string.

```cs
Base64.encode("test"); // 'dGVzdA=='
```

### Base64.decode(string)

Base64 decode a given string.

```cs
Base64.decode("dGVzdA=="); // 'test'
```
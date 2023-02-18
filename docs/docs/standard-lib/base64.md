---
layout: default
title: Base64
nav_order: 10
parent: Standard Library
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

### Base64.encode(String) -> String

Base64 encode a given string.

```cs
Base64.encode("test"); // 'dGVzdA=='
```

### Base64.decode(String) -> String

Base64 decode a given string.

```cs
Base64.decode("dGVzdA=="); // 'test'
```
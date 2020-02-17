---
layout: default
title: JSON
nav_order: 14
---

# JSON
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

### Parse

Parses a JSON string and turns it into a valid Dictu datatype.

```js
JSON.parse('true'); // true
JSON.parse('{"test": 10}'); // {"test": 10}
JSON.parse('[1, 2, 3]'); // [1, 2, 3]
```

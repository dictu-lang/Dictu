---
layout: default
title: Inspect
nav_order: 14
parent: Standard Library
---

# Inspect
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Inspect
To make use of the Inspect module an import is required.

```js
import Inspect;
```

### Inspect.getLine(number: count -> optional)

This gives you the line number within the file that the function was called from.

The optional argument passed is the amount of frames to traverse back up. If this number exceeds the
frame count an error is raised.

```cs
Inspect.getLine(); // 1
Inspect.getLine(1000); // Optional argument passed to getLine() exceeds the frame count.
```
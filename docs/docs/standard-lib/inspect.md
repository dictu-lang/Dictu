---
layout: default
title: Inspect
nav_order: 11
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

### Inspect.getFrameCount() -> Number

This gives you the current frame count of the VM at the point of calling.

Note: This is 0-based.

```cs
Inspect.getFrameCount(); // 0

def test() {
    print(Inspect.getFrameCount());
}

test(); // 1
```

### Inspect.getLine(Number: count -> optional) -> Number

This gives you the line number within the file that the function was called from.

The optional argument passed is the amount of frames to traverse back up. If this number exceeds the
frame count an error is raised.

```cs
Inspect.getLine(); // 1
Inspect.getLine(1000); // Optional argument passed to getLine() exceeds the frame count.
```

### Inspect.getFile(Number: count -> Optional) -> String

This gives you the name of the file that the function was called from.

The optional argument passed is the amount of frames to traverse back up. If this number exceeds the
frame count an error is raised.

```cs
Inspect.getFile(); // repl
Inspect.getFile(); // myFile.du
```

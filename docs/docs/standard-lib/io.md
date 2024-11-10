---
layout: default
title: IO
nav_order: 11
parent: Standard Library
---

# IO
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## IO

To make use of the IO module an import is required.

```js
import IO;
```

### Constants

| Constant   | Description                         |
| ---------- | ----------------------------------- |
| IO.devNull | Provides access to the null device. |

### IO.print(...values) -> Nil

Prints a given list of values to stdout.

```cs
IO.print(0);
// 0
```

### IO.println(...values) -> Nil

Prints a given list of values to stdout with an appended newline character.

```cs
IO.println("Dictu!");
// Dictu!
```

### IO.eprint(...values) -> Nil

Prints a given list of values to stderr.

```cs
IO.eprint(0);
// 0
```

### IO.eprintln(...values) -> Nil

Prints a given list of values to stderr with an appended newline character.

```cs
IO.eprintln("Dictu!");
// Dictu!
```


### IO.copyFile(String: src, String: dst) -> Result\<Nil>

Copies the contents from the source file to the destination file.

Returns a Result type and on success will unwrap to nil.

```cs
IO.copyFile(src, dst);
```

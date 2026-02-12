---
layout: default
title: Term
nav_order: 25
parent: Standard Library
---

# Term
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Term

To make use of the Term module an import is required.

```js
import Term;
```

### Term.isatty(Number) -> Boolean

Returns a boolean indicating whether the file descriptor passed is attached to a tty.

```cs
Term.isatty(0);
```

### Term.getSize() -> Number

Returns the number of rows, columns, horizontal and vertical pixels of the attached terminal.

```cs
Term.getSize();
print(Term.getSize());
// {"rows": 13, "columns": 145, "horizontal_pixels": 145, "vertical_pixels": 145}
```

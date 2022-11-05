---
layout: default
title: Term
nav_order: 16
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

```cs
import Term;
```

### Term.isatty(number)

Returns a boolean indicating whether the file descriptor passed is attached to a tty.

```cs
Term.isatty(0);
```

### Term.getSize()

Returns the number of rows, columns, horizantal and vertical pixels of the attached terminal.

```cs
Term.getSize();
print(Term.getSize());
// {"rows": 13, "columns": 145, "horizantal_pixels": 145, "vertical_pixels": 145}
```

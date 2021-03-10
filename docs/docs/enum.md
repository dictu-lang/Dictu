---
layout: default
title: Enums
nav_order: 10
---

# Enums
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Enums

Enums are a collection of constants which can be accessed via a name rather than
an index to document intent. Unlike other languages, enums in Dictu must be assigned
to a value when declaring the enum and no automatic value will be generated.

```cs
enum Test {
    a = 0,
    b = 1,
    c = 2
}

print(Test.a); // 0
```

Enums in Dictu also do not care about the value being stored within the enum, so
if you wanted to create a heterogeneous enum, Dictu will not stop you.

```cs
enum HeterogeneousEnum {
    a = 0,
    b = "string",
    c = def () => 10
}
```
---
layout: default
title: Style-Guide
nav_order: 9
---

# Style-Guide
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Introduction

## Formatting

## Commentary

## Names

## Module Names

When a module is imported, the module name becomes an accessor for the contents. After

```cs
import System;
```

the importing package can talk about `System.uname()`. The convention for naming is for modules

## Mixed Caps

The convention in Dictu is to use `MixedCaps` or `mixedCaps` to write multiword names. The only exception to this rule is when defining constants. Constants should be defined using all upper case in snake case.

## Control Structures

Dictu provides a number of [control structures](https://dictu-lang.com/docs/control-flow/) allowing for robust functionality. These strcutures should be formatted as follows:

```cs
if () {
    // code
}
```

```cs
while () {
    // code
}

while {
    // code
}
```

```cs
for (...) {
    // code
}
```
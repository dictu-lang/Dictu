---
layout: default
title: Style-Guide-Best-Practices
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

Depending on how a module is imported, either module name becomes the accessor or the item is accessed directly. Modules should have short names and be simple nouns describing the contents. For example: `Sockets`, `HTTP`.

```cs
import System;
```

the importing package can talk about `System.uname()`. The convention for naming is for modules

## Mixed Caps

The naming convention in Dictu is to use mixed caps, Camel Case and Pascal Case, to write multiword names. The only exception to this rule is when defining constants. 

## Constants 

Constants should be defined using all upper case in snake case.

## Control Flow

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
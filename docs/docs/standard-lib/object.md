---
layout: default
title: Object
nav_order: 17
parent: Standard Library
---

# Object
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Object

To make use of the Object module an import is required.

```cs
import Object;
```

### Object.getClassRef(String) -> Result\<Object>

This method will attempt to get a class reference from the class name provided as a string.

Returns a Result and unwraps to an Object upon success.

**NOTE:** This currently only works if the class is defined in the global scope

```cs
class Test {}

Object.getClassRef("Test").unwrap(); // <Cls Test>
```

### Object.createFrom(String) -> Result\<Object>

This method will attempt to create a new object from the class name provided as a string.

Returns a Result and unwraps to an Object upon success.

**NOTE:** This currently only works if the class is defined in the global scope

```cs
class Test {}

Object.createFrom("Test").unwrap(); // <Test instance>
```

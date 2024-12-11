---
layout: default
title: Importlib
nav_order: 10
parent: Standard Library
---

# Inspect
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Importlib
To make use of the Importlib module an import is required.

```js
import Importlib;
```

### Importlib.include(String) -> Module

This allows you to dynamically import a module. 

Note: The path is relative to the file include() is called in.

```js
// someFile.du
class SomeClass {
    hello() {
        print("Hello!");
    }
}
```

```js
// main.du
const SomeModule = Importlib.include('someFile.du'); // <Module someFile.du>

SomeModule.SomeClass().hello();
```

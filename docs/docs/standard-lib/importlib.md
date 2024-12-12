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

### Importlib.include(String, Bool: reload -> Optional) -> Module

This allows you to dynamically import a module. It also allows you to reload a module that was previously imported.

Note: The path is relative to the file include() is called in.
Note: Reloading an already imported module will mutate all references to the previously imported module.

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

---
layout: default
title: Importlib
nav_order: 10
parent: Standard Library
---

# Importlib
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

### Importlib.include(String, Boolean: reload -> Optional) -> Module

Dynamically imports and executes a Dictu source file, returning its top-level declarations as a Module. If the module has already been imported, the cached version is returned unless `reload` is set to `true`.

**Note:** The path is relative to the file that `include()` is called from.

**Note:** Reloading a previously imported module will mutate all existing references to that module.

```js
// someFile.du
class SomeClass {
    hello() {
        print("Hello!");
    }
}

const version = 1;
```

```js
// main.du
const SomeModule = Importlib.include("someFile.du");

SomeModule.SomeClass().hello(); // "Hello!"
print(SomeModule.version); // 1
```

Nested includes are supported. Each included file can itself call `Importlib.include()` to load further modules.

```js
// level2.du
const items = [1, 2, 3];
```

```js
// level1.du
import Importlib;

const Level2 = Importlib.include("level2.du");
```

```js
// main.du
import Importlib;

const Level1 = Importlib.include("level1.du");
print(Level1.Level2.items); // [1, 2, 3]
```

To force a module to be recompiled and re-executed, pass `true` as the second argument.

```cs
const MyModule = Importlib.include("config.du");

// Later, after config.du has been modified on disk:
const ReloadedModule = Importlib.include("config.du", true);
```

---
layout: default
title: Env
nav_order: 5
parent: Standard Library
---

# Env
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Env

To make use of the Env module an import is required.

```cs
import Env;
```

### Constants

| Constant        | Description               |
|-----------------|---------------------------|
| Env.errno       | Number of the last error. |

### Env.strerror(number: error -> optional)
Get the string representation of an error.
An optional error status can be passed, otherwise the default is Env.errno.
It returns a string that describes the error.

```cs
print(Env.strerror());
```

### Env.get(string)

Get an environment variable. If the ENV var does not exist nil is returned and sets errno accordingly,
otherwise a string value is returned.

```cs
Env.get("bad key!"); // nil
Env.get("valid key"); // "value"
```

### Env.set(string, value)

Change or add an environment variable. You can clear an ENV var by passing a `nil` value.
When setting an ENV var the key must be a string and the value must be either a string or nil.
Returns 0 upon success or -1 otherwise and sets Env.errno accordingly.

```cs
Env.set("key", "test");
Env.set("key", nil); // Remove env var
Env.set("key", 10); // set() arguments must be a string or nil.
```

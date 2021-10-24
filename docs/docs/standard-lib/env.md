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

### Env.get(string, string: defaultValue -> optional)

Get an environment variable. `.get()` will return a string if a valid environment variable is found otherwise nil.

If default value is passed that will be returned if the specified environment variable could not be found.

```cs
Env.get("bad key!"); // nil
Env.get("valid key"); // "value"
Env.get("bad key!", "default value!!!"); // "default value!!!"
```

### Env.set(string, value)

Change or add an environment variable. You can clear an environment variable by passing a `nil` value.
When setting an environment variable the key must be a string and the value must be either a string or nil.
Returns a Result type and on success will unwrap to nil.

```cs
Env.set("key", "test");
Env.set("key", nil); // Remove env var
Env.set("key", 10); // set() arguments must be a string or nil.
```

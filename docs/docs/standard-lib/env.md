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

### Env.get(String, String: defaultValue -> Optional) -> String -> Nil

Get an environment variable. `.get()` will return a string if a valid environment variable is found otherwise nil.

If default value is passed that will be returned if the specified environment variable could not be found.

```cs
Env.get("bad key!"); // nil
Env.get("valid key"); // "value"
Env.get("bad key!", "default value!!!"); // "default value!!!"
```

### Env.set(String, String)

Change or add an environment variable. You can clear an environment variable by passing a `nil` value.
When setting an environment variable the key must be a string and the value must be either a string or nil.
Returns a Result type and on success will unwrap to nil.

```cs
Env.set("key", "test");
Env.set("key", nil); // Remove env var
Env.set("key", 10); // set() arguments must be a string or nil.
```

### Env.clearAll()

Clears all set environment variables.

Note: This is not available on Windows systems.

```cs
Env.clearAll();
```

### Env.readFile(String: path -> Optional) -> Result

To read environment variables from a file this helper method is provided.
By default it will attempt to read `.env` unless a different path is supplied.
Returns a Result type and on success will unwrap to nil.

Note: You are able to have comments in the `.env` file via the use of `#` (both in-line and first character).

```env
# This is a comment
TEST=10
TESTING=100 # In-line comment
```

```cs
Env.readFile(); // <Result Suc>

print(Env.get("TEST")); // 10
print(Env.get("TESTING")); // 100
```
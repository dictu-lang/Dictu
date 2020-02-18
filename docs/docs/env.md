---
layout: default
title: Env
nav_order: 12
---

# Env
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

### Env.get(string)

You can access ENV vars through the use of the Env.get() method. If the ENV var does not exist nil is returned, other wise
a string value is returned.

```js
Env.get("bad key!"); // nil
Env.get("valid key"); // "value"
```

### Env.set(string, value)

You can set ENV vars through the use of the Env.set() method. You can also clear an ENV var by passing a `nil` value.
When setting an ENV var the key must be a string and the value must be either a string or nil;

```js
Env.set("key", "test");
Env.set("key", nil); // Remove env var
Env.set("key", 10); // set() arguments must be a string or nil.
```
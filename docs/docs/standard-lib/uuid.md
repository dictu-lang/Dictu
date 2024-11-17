---
layout: default
title: UUID
nav_order: 27
parent: Standard Library
---

# UUID
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## UUID

To make use of the UUID module an import is required.

```js
import UUID;
```

### UUID.generate() -> Result\<String>

Returns a Result value with a string representation of the UUID on success or an Error on failure. This function attempts to use `/dev/urandom` if available but if it's not, it uses alterntive means of generating randomness.

Note: This is not available on Windows systems.

```cs
const uuid = UUID.generate(0).unwrap();
print(uuid);
// a9c313d8-5bdb-4537-af9a-0b08be1387fb
```

### UUID.generateRandom() -> Result\<String>

Returns a Result value with a string representation of the UUID on success or an Error on failure. This function forces the use of the all-random UUID format.

Note: This is not available on Windows systems.

```cs
const uuid = UUID.generateRandom().unwrap();
print(uuid);
// 95356011-f08c-46d9-9335-d5b988682211
```

### UUID.generateTime() -> Result\<String>

Returns a Result value with a string representation of the UUID on success or an Error on failure. This function forces the use of the alternative algorithm which uses the current time and local MAC address (if available).

Note: This is not available on Windows systems.

```cs
const uuid = UUID.generateTime().unwrap();
print(uuid);
// 55edea51-e712-4352-a8c4-bb4eaa69d49d
```

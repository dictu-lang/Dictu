---
layout: default
title: Hashlib
nav_order: 7
parent: Standard Library
---

# Hashlib
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Hashlib
To make use of the Hashlib module an import is required.

```js
import Hashlib;
```

### Hashlib.sha256(String) -> String

Hashes a given string using the SHA-256 algorithm.

```cs
Hashlib.sha256("Dictu"); // 889bb2f43047c331bed74b1a9b309cc66adff6c6d4c3517547813ad67ba8d105
```

### Hashlib.hmac(String: key, String: payload, Boolean: raw -> Optional) -> String

Generate a HMAC using the SHA-256 algorithm. The `raw` optional argument determines whether the output
will be in raw bytes or whether it will be in a string format, default is string format (false).

```cs
Hashlib.hmac("supersecretkey", "my message"); // 2dd2a27b79858666b9d9ba3cf73f1c84f5722415f9543d007fa73e081d02483a
Hashlib.hmac("supersecretkey", "my message", true); // <bytes>
```

### Hashlib.bcrypt(String, Number: rounds -> Optional) -> String

Hashes a given string using the bcrypt algorithm.
The rounds optional argument is the amount of rounds used to generate the hashed string, default is 8.

```cs
Hashlib.bcrypt("my message"); // $2b$08$mkI2fcaukY0XX3qlpdtBgeXq7pAUr2bUw4Z1OkmncuibJ0aHAyLRS
Hashlib.bcrypt("my message", 2); // $2b$04$pNdfFWIV2r31vmQmJOSwNe/CHupV/wpOHmmwsDjCZi45w8ttjA/WW
```

### Hashlib.bcryptVerify(String: plainText, String: hash) -> Boolean

This verifies a given bcrypt hash matches the plaintext input with a boolean return value.
The comparison method used is timing safe.

```cs
Hashlib.bcryptVerify("my message", "$2b$08$mkI2fcaukY0XX3qlpdtBgeXq7pAUr2bUw4Z1OkmncuibJ0aHAyLRS"); // true
Hashlib.bcryptVerify("my message", "$2b$04$pNdfFWIV2r31vmQmJOSwNe/CHupV/wpOHmmwsDjCZi45w8ttjA/WW"); // true
Hashlib.bcryptVerify("my message", "wrong"); // false
```

### Hashlib.verify(String: hash, String: hash) -> Boolean

Timing safe hash comparison. This should always be favoured over normal string comparison.

```cs
Hashlib.verify(Hashlib.sha256("Dictu"), "889bb2f43047c331bed74b1a9b309cc66adff6c6d4c3517547813ad67ba8d105"); // true
Hashlib.verify(Hashlib.sha256("Dictu"), "wrong"); // false
```

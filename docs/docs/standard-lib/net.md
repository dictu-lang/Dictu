---
layout: default
title: Net
nav_order: 15
parent: Standard Library
---

# Net
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Net

To make use of the Net module an import is required.

```js
import Net;
```

### Net.parseIp4(String) -> Result\<List>

parseIp4 takes a string representation of an IP4 address and returns a Result that unwraps to list containing an entry for each octet.

```cs
Net.parseIp4("10.0.0.2").unwrap();
// [10, 0, 0, 2]
```

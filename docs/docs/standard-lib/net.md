---
layout: default
title: IO
nav_order: 13
parent: Standard Library
---

# IO
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Net

To make use of the Net module an import is required.

```cs
import Net;
```

### Net.parseIp4(String) -> Nil

parseIp4 takes a string representation of an IP4 address and returns a Result that unwraps to list containing an entry for each octet.

```cs
Net.parseIp4("10.0.0.2");
// [10, 0, 0, 2]
```

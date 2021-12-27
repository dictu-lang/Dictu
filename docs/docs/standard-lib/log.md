---
layout: default
title: Log
nav_order: 16
parent: Standard Library
---

# Log
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Log

To make use of the Log module an import is required.

```cs
import Log;
```

### Constants

| Constant   | Description                                                           |
| ---------- | --------------------------------------------------------------------- |
| Log.stdout | The default file descriptor where a process can write output.         |
| Log.stderr | The default file descriptor where a process can write error messages. |
| Log.stdin  | An input stream where data is sent to and read by a program.          |

### Log.print(string)

```cs
Log.print("something extremely interesting"); 2021/11/27 11:49:14 something extremely interesting
```

### Log.println(string)

```cs
Log.println("hello, world! println"); // 2021/11/27 11:49:14 hello, world! print
```

### Log.new(number)

Create a new instance of a logger.

```cs
var log = Log.new(Log.stdout).unwrap();
```

### log.setPrefix(string)

A prefix can be set on the logger that will be included in the output just before the user provided content.

```cs
log.setPrefix("prefix");
```

### log.unsetPrefix(string)

Remove the prefix on the logger. This is a noop if there was no prefix previously set.

```cs
log.unsetPrefix();
```
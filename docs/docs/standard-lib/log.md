---
layout: default
title: Log
nav_order: 13
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

```js
import Log;
```

### Constants

| Constant   | Description                                                           |
| ---------- | --------------------------------------------------------------------- |
| Log.stdout | The default file descriptor where a process can write output.         |
| Log.stderr | The default file descriptor where a process can write error messages. |
| Log.stdin  | An input stream where data is sent to and read by a program.          |

### Log.print(String)

```cs
Log.print("something extremely interesting"); // 2021/11/27 11:49:14 something extremely interesting
```

### Log.println(string)

```cs
Log.println("hello, world! println"); // 2021/11/27 11:49:14 hello, world! print
```

### Log.fatal(String)

Log the given output and exit the program with an exit code of 1.

```cs
Log.fatal("we've met a tragic end"); // 2021/11/27 11:49:14 we've met a tragic end
```

### Log.fatalln(String)

Log the given output with a new line and exit the program with an exit code of 1.

```cs
Log.fatalln("hello, world! fatalln"); // 2021/11/27 11:49:14 hello, world! print
```

### Log.new(Number)

Create a new instance of a logger.

```cs
const log = Log.new(Log.stdout).unwrap();
```

### log.setPrefix(String)

A prefix can be set on the logger that will be included in the output just before the user provided content.

```cs
log.setPrefix("prefix");
```

### log.unsetPrefix(String)

Remove the prefix on the logger. This is a noop if there was no prefix previously set.

```cs
log.unsetPrefix();
```

### log.print(String)

```cs
Log.print("something extremely interesting"); // 2021/11/27 11:49:14 something extremely interesting
```

### log.println(String)

```cs
Log.println("hello, world! println"); // 2021/11/27 11:49:14 hello, world! print
```

### log.fatal(String)

Log the given output and exit the program with an exit code of 1.

```cs
log.fatal("we've met a tragic end"); // 2021/11/27 11:49:14 we've met a tragic end
```

### log.fatalln(String)

Log the given output with a new line and exit the program with an exit code of 1.

```cs
log.fatalln("hello, world! fatalln"); // 2021/11/27 11:49:14 hello, world! print
```

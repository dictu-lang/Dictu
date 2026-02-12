---
layout: default
title: Process
nav_order: 18
parent: Standard Library
---

# Process
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Process

To make use of the Process module an import is required.

```js
import Process;
```

### Process.exec(List) -> Result\<Nil>

Executing an external process can be done via `.exec`. Unlike `.run()` exec does not wait for the process
to finish executing, so it is only useful for circumstances where you wish to "fire and forget".

`.exec()` expects a list as a parameter which is the command and any arguments for the command as individual list elements, which all must be strings.
It will return a Result that unwraps to `nil` on success.

```cs
Process.exec(["ls", "-la"]);
```

### Process.run(List, Boolean: captureOutput -> Optional) -> Result\<Nil>

Similar to `.exec()` except this **will** wait for the external process to finish executing.

`.run()` expects a list as a parameter which is the command and any arguments for the command as individual list elements,
and it will return a Result that unwraps to `nil` on success.

If the external process writes to stdout and you wish to capture the output you can pass an optional boolean argument to
`.run()`, this will instead make the Result unwrap to a string of the captured output.

```cs
Process.run(["ls", "-la"]).unwrap();
print(Process.run(["echo", "test"], true).unwrap()); // 'test'
```

### Process.kill(Number, Number -> Optional) -> Result\<Nil>

kill receives a process ID number and an optional signal number and attempts to kill the process associated with the given pid. If no signal is provided, SIGKILL is used.

```cs
const res = Process.kill(709871);
// 0
```

```cs
const res = Process.kill(709871, Process.SIGTERM).unwrap();
// 0
```

**Note:** On Windows, `kill` only takes the PID as the argument.

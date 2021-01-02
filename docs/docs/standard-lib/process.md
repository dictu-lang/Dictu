---
layout: default
title: Process
nav_order: 12
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

```cs
import Process;
```

### Process.exec(list)

Executing an external process can be done via `.exec`. Unlike `.run()` exec does not wait for the process
to finish executing, so it is only useful for circumstances where you wish to "fire and forget".

`.exec()` expects a list as a parameter which is the command and any arguments for the command as individual list elements, which all must be strings.
It will return a Result that unwraps to `nil` on success.

```cs
Process.exec(["ls", "-la"]);
```

### Process.run(list, boolean: captureOutput -> optional)

Similar to `.run()` except this **will** wait for the external process to finish executing.

`.run()` expects a list as a parameter which is the command and any arguments for the command as individual list elements,
and it will return a Result that unwraps to `nil` on success.

If the external process writes to stdout and you wish to capture the output you can pass an optional boolean argument to
`.run()`, this will instead make the Result unwrap to a string of the captured output.

```cs
Process.run(["ls", "-la"]).unwrap();
print(Process.run(["echo", "test"], true).unwrap()); // 'test'
```
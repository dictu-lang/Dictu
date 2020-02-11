---
layout: default
title: System
nav_order: 13
---

# System
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

### Sleep

Sleep pauses execution of the program for a given amount of time in seconds

```js
System.sleep(3); // Pauses execution for 3 seconds
```

### Clock

Returns number of clock ticks since the start of the program, useful for benchmarks.

```js
System.clock();
```

### Time

Returns UNIX timestamp.

```js
System.time();
```

### Collect

Manually trigger a garbage collection.

```js
System.collect();
```

### Exit

When you wish to prematurely exit the script with a given exit code

```js
System.exit(10);
```

Shell
```bash
$ echo $?; // 10
```
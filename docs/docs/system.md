---
layout: default
title: System
nav_order: 14
---

# System
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

### Constants

| Constant        | Description                                                                                       |
|-----------------|---------------------------------------------------------------------------------------------------|
| System.argv     | The list of command line arguments. The first element of the argv list is always the script name. |
| System.errno    | Number of the last error.                                                                         |
| System.platform | This string identifies the underlying system platform.                                            |
| System.S_IRWXU  | Read, write, and execute by owner.                                                                |
| System.S_IRUSR  | Read by owner.                                                                                    |
| System.S_IWUSR  | Write by owner.                                                                                   |
| System.S_IXUSR  | Execute by owner.                                                                                 |
| System.S_IRWXG  | Read, write, and execute by group.                                                                |
| System.S_IRGRP  | Read by group.                                                                                    |
| System.S_IWGRP  | Write by group.                                                                                   |
| System.S_IXGRP  | Execute by group.                                                                                 |
| System.S_IRWXO  | Read, write, and execute by others.                                                               |
| System.S_IROTH  | Read by others.                                                                                   |
| System.S_IWOTH  | Write by others.                                                                                  |
| System.S_IXOTH  | Execute by others.                                                                                |
| System.S_ISUID  | Set user ID on execution.                                                                         |
| System.S_ISGID  | Set group ID on execution.                                                                        |

### System.strerror(number: error -> optional)
Get the string representation of an error.
An optional error status can be passed, otherwise the default is System.errno.
It returns a string that describes the error.

```js
print(System.strerror());
```

### System.mkdir(string, number: mode -> optional)

Make directory.
Returns 0 upon success or -1 otherwise and sets System.errno accordingly.

It can take an optional number argument that specifies the mode. If a mode is not passed, the directory will be created with `0777` permissions.

The actual permissions is modified by the process umask, which typically is S_IWGRP|S_IWOTH (octal 022).

```js
var
  S_IRWXU = System.S_IRWXU,
  S_IRGRP = System.S_IRGRP,
  S_IXGRP = System.S_IXGRP,
  S_IROTH = System.S_IROTH,
  S_IXOTH = System.S_IXOTH;

System.mkdir(dir, S_IRWXU|S_IRGRP|S_IXGRP|S_IXOTH|S_IROTH);
```

### System.rmdir(string)

Remove directory.

Returns 0 upon success or -1 otherwise and sets System.errno accordingly.

```js
System.rmdir(dir);
```

### System.remove(string)

Delete a file from filesystem.

Returns 0 upon success or -1 otherwise and sets System.errno accordingly.

```js
System.remove(file);
```

### System.getpid()

Returns the process ID (PID) of the calling process.

```js
System.getpid();
```

### System.getppid()

Returns the process ID of the parent of the calling process

```js
System.getppid();
```

### System.getuid()

Returns the real user ID of the calling process.

```js
System.getuid();
```

### System.geteuid()

Returns the effective user ID of the calling process.

```js
System.geteuid();
```

### System.getgid()

Returns the real group ID of the calling process.

```js
System.getgid();
```

### System.getegid()

Returns the effective group ID of the calling process.

```js
System.getegid();
```

### System.getCWD()

Get the current working directory of the Dictu process.

Returns a string upon success or nil otherwise and sets System.errno accordingly.

```js
System.getCWD(); // '/Some/Path/To/A/Directory'
```

### System.setCWD(string)

Set current working directory of the Dictu process.

Returns 0 upon success or -1 otherwise and sets System.errno accordingly.

```js
if (System.setCWD('/') == -1) {
  print ("failed to change directory");
}
```

### System.sleep(number)

Sleep pauses execution of the program for a given amount of time in seconds.

```js
System.sleep(3); // Pauses execution for 3 seconds
```

### System.clock()

Returns number of clock ticks since the start of the program, useful for benchmarks.

```js
System.clock();
```

### System.time()

Returns UNIX timestamp.

```js
System.time();
```

### System.collect()

Manually trigger a garbage collection.

```js
System.collect();
```

### System.exit(number)

When you wish to prematurely exit the script with a given exit code.

```js
System.exit(10);
```

Shell
```bash
$ echo $?; // 10
```

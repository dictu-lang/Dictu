---
layout: default
title: System
nav_order: 1
parent: Standard Library
---

# System
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## System
To make use of the System module an import is required.

```js
import System;
```

### Constants

| Constant        | Description                                                                                       |
| --------------- | ------------------------------------------------------------------------------------------------- |
| System.argv     | The list of command line arguments. The first element of the argv list is always the script name. |
| System.platform | This string identifies the underlying system platform.                                            |
| System.version  | Dictionary containing Dictu major, minor and patch versions.                                      |
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
| System.F_OK     | Test for existence.                                                                               |
| System.X_OK     | Test for execute permission.                                                                      |
| System.W_OK     | Test for write permission.                                                                        |
| System.R_OK     | Test for read permission.                                                                         |

### System.mkdir(string, number: mode -> optional)

Make directory.
Returns a Result type and on success will unwrap nil.

It can take an optional number argument that specifies the mode. If a mode is not passed, the directory will be created with `0777` permissions.

The actual permissions is modified by the process umask, which typically is S_IWGRP\|S_IWOTH (octal 022).

```cs
var
  S_IRWXU = System.S_IRWXU,
  S_IRGRP = System.S_IRGRP,
  S_IXGRP = System.S_IXGRP,
  S_IROTH = System.S_IROTH,
  S_IXOTH = System.S_IXOTH;

System.mkdir(dir, S_IRWXU|S_IRGRP|S_IXGRP|S_IXOTH|S_IROTH);
```

### System.access(string, number)

Check user's permissions for a file
Returns a Result type and on success will unwrap nil.

**Note:** This method and the F_OK\|X_OK\|W_OK\|R_OK constants are not available on windows systems.

```cs
var F_OK = System.F_OK;

System.access("/", F_OK);
```

### System.rmdir(string)

Remove directory.

Returns a Result type and on success will unwrap nil.

```cs
System.rmdir(dir);
```

### System.remove(string)

Delete a file from filesystem.

Returns a Result type and on success will unwrap nil.

```cs
System.remove(file);
```

### System.getpid()

Returns the process ID (PID) of the calling process as a number.

**Note:** This is not available on windows systems.

```cs
System.getpid();
```

### System.getppid()

Returns the process ID of the parent of the calling process as a number.

**Note:** This is not available on windows systems.

```cs
System.getppid();
```

### System.getuid()

Returns the real user ID of the calling process as a number.

**Note:** This is not available on windows systems.

```cs
System.getuid();
```

### System.geteuid()

Returns the effective user ID of the calling process as a number.

**Note:** This is not available on windows systems.

```cs
System.geteuid();
```

### System.getgid()

Returns the real group ID of the calling process as a number.

**Note:** This is not available on windows systems.

```cs
System.getgid();
```

### System.getegid()

Returns the effective group ID of the calling process as a number.

**Note:** This is not available on windows systems.

```cs
System.getegid();
```

### System.getCWD()

Get the current working directory of the Dictu process.

Returns a Result type and on success will unwrap a string.

```cs
System.getCWD().unwrap(); // '/Some/Path/To/A/Directory'
```

### System.setCWD(string)

Set current working directory of the Dictu process.

Returns a Result type and on success will unwrap nil.

```cs
if (!System.setCWD('/').success()) {
    print("failed to change directory");
}
```

### System.sleep(number)

Sleep pauses execution of the program for a given amount of time in seconds.

```cs
System.sleep(3); // Pauses execution for 3 seconds
```

### System.clock()

Returns number of clock ticks since the start of the program as a number, useful for benchmarks.

```cs
System.clock();
```

### System.time()

Returns UNIX timestamp as a number.

```cs
System.time();
```

### System.collect()

Manually trigger a garbage collection.

```cs
System.collect();
```

### System.exit(number)

When you wish to prematurely exit the script with a given exit code.

```cs
System.exit(10);
```

Shell
```bash
$ echo $?; // 10
```

### System.chmod(string, string)

Set the permissions on a file or directory.

```cs
System.chmod("/usr/local/share", "755");
```

### System.chown(string, number, number)

Set the ownership of a file or directory with the given path, uid, and gid.

Note: This is not available on Windows systems.

```cs
System.chown("/path/to/file", 0, 0);
```

### System.uname()

Returns the name and version of the system along with operating system and hardware information.

Note: This is not available on Windows systems.

```cs
System.uname();
```

### System.mkdirTemp(string: directory_template -> optional)

Makes a temporary directory. If an empty string is given, the temporary directory's name will be a random string created in the current working directory. If a string is passed in, the temporary directory will be created with that name in the current working directory.

The directory template passed in **must** end with "XXXXXX".

Returns a Result type and on success will unwrap to a string containing the created directory name.

Note: This is not available on Windows systems.

```cs
System.mkdirTemp().unwrap(); // "VOO16s"
System.mkdirTemp("test_XXXXXX").unwrap(); // "test_0bL2qS"
```

### System.copyFile(string: src, string: dst)

Copies the contents from the source file to the destination file.

Returns a Result type and on success will unwrap to nil.

```cs
System.copyFile(src, dst);
```

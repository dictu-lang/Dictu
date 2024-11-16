---
layout: default
title: Files
nav_order: 12
---

# Files
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## Files

### Opening files

Opening files is very easy in Dictu. Syntax is `with(<file name>, <open mode>) {}`. Inside the `with` block, `file` is declared
as a constant for you, and is the file object which has just been opened. Dictu will handle closing the file for you and happens
when you leave the with scope automatically. Note, the path when opening files is relative to the process **not** the current script running.

| Open Mode | Description                                                                                                                |
| --------- | -------------------------------------------------------------------------------------------------------------------------- |
| r         | Opens a file for reading, the file must exist already.                                                                     |
| w         | Opens a file for writing, if a file does not exist one is created, else existing file is overwritten.                      |
| rb        | Opens a file for reading binary, the file must exist already.                                                              |
| wb        | Opens a file for writing in binary mode, if a file does not exist one is created, else existing file is overwritten.       |
| a         | Opens a file for appending, if a file does not exist one is created, else appends text to the end of a file.               |
| r+        | Opens a file for updating (read + write), the file must exist already.                                                     |
| w+        | Opens a file for updating (read + write), if a file does not exist one is created, else existing file is overwritten       |
| a+        | Opens a file for updating (read + write), if a file does not exist one is created, else appends text to the end of a file. |

```cs
with("test.txt", "r") {
    // file var is passed in here
}

// file is out of scope, and will the file will be closed for you
```

### Custom name
It is possible to append `as <name>` before the opening `{` in order to customise the Name of the variable within the block.
```cs
with("test.txt", "r") as myfile {
    // file var is passed in here as myfile, NOT file.
}
```

### Writing to files

There are two methods available when writing to a file: `write()` and `writeLine()`. `write()` simply writes strings to a file, `writeLine()` is exactly the same, except it appends a newline to the passed in string. Both functions return the amount of characters wrote to the file.

```cs
with("test.txt", "w") {
    // file var is passed in here
    file.writeLine("Hello!"); // 6
    file.write("Hi"); // 2
}

// file is out of scope, and will be closed for you here
```

### Reading files

There are two methods available when reading files: `read()` and `readLine()`. `read()` reads the entire file, and returns its content as a string. `readLine()` will read the file up to a new line character.

`readline()` can also take an optional argument for the buffer size.

```cs
// Read entire file
with("test.txt", "r") {
    print(file.read());
}
```

```cs
// Read a file line by line
with("test.txt", "r") {
    var line;
    // When you reach the end of the file, nil is returned
    while((line = file.readLine()) != nil) {
        print(line);
    }
}

with("test.txt", "r") {
    var line;
    // When you reach the end of the file, nil is returned
    while((line = file.readLine(10)) != nil) {
        print(line); // Will only read 10
    }
}
```

Another method which may come in useful when reading files is `seek()`. `seek()` allows you to move the file cursor so you can re-read a file, for example, without closing the file and reopening.

### file.Seek(Number, Number: from  -> Optional)
Both arguments passed to seek need to be of numeric value, however the `from` argument is optional.
The first argument (offset) is the amount of characters you wish to move from the cursor position (negative offset for seeking backwards).
The second argument (from) is for controlling where the cursor will be within the file, options are 0, 1 or 2.
- 0 => Seek from the start of the file.
- 1 => Seek from the current cursor position.
- 2 => Seek from the end of the file.

If the `from` parameter is not supplied, `seek()` will occur from the beginning of the file.

```cs
with("test.txt", "r") {
    print(file.read());
    file.seek(0); // Go back to the start of the file
    print(file.read());
    file.seek(-5, 1); // Go back 5 from current position
    print(file.read());
    file.seek(0); // Go to the start of the file
    file.seek(-5, 2); // Go to the end of the file, and then 5 places back
    print(file.read());
}
```
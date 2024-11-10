---
layout: default
title: Argparse
nav_order: 1
parent: Standard Library
---

# Argparse
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Argparse

To make use of the Argparse module an import is required.

```js
from Argparse import Parser;
```

### New Parser(String, String, String -> Optional) -> Parser

To create a new parser instance, call the `Parser` class with the 2 required string arguments; name, description.

You can optionally pass in a user generated usage for the Parser

```cs
const parser = Parser("prog_name", "Program to do all the things");
// 
const parser = Parser("prog_name", "Program to do all the things", "User defined usage string");
```

### Parser.addString(String, String, Bool, string -> Optional)

To add a new string argument, call the method below with at least the 3 required arguments; name, description, and boolean indicating the flag is required. A fourth argument can be passed to give the parser a custom name for the given flag.

```cs
parser.addString("-b", "birth month", false, "month");
```

### Parser.addNumber(String, String, Bool, String -> Optional)

To add a new number argument, call the method below with at least the 3 required arguments; name, description, and boolean indicating the flag is required. A fourth argument can be passed to give the parser a custom name for the given flag.

```cs
parser.addNumber("--port", "server port", false);
```

### Parser.addBool(String, String, Bool, String -> Optional)

To add a new bool argument, call the method below with at least the 3 required arguments; name, description, and boolean indicating the flag is required. A fourth argument can be passed to give the parser a custom name for the given flag.

```cs
parser.addBool("-s", "run securely", true, "secure");
```

### Parser.addList(String, String, Bool, String -> Optional)

To add a new list argument, call the method below with at least the 3 required arguments; name, description, and boolean indicating the flag is required. A fourth argument can be passed to give the parser a custom name for the given flag.

```cs
parser.addList("-u", "active users", true, "users");
```

### Parser.usage() -> String

The Parser class will create a helpful output based on the arguments added to the parser class for you, this can be used to populate a `--help` argument.

```cs
const parser = Parser("Code!", "Some code");
parser.addBool("--myOption", "Some useful boolean option", false);
print(parser.usage());

// Output
usage: Code!
    Some code

    --myOption    Some useful boolean option

```

### Parser.parse() -> Result\<Args>

The `parse` method needs to be called to process the given flags against the configured flags. `parse` returns a `Result` value that, on success, will need to be unwrapped to access an instance of the `Args` class.

```cs
const args = parser.parse().match(
    def (result) => result,
    def (error) => {
        print(error);
        System.exit(1);
    }
);
```

The value of type `Args` will be instantiated with fields matching the configured flags or the given metavar names.

If the option was not marked as required and a value for that given argument was not passed then it will have a `nil` value.

```cs
const parser = Parser("Code!", "Some code");
parser.addString("--option", "Some useful string option", false);
parser.addString("--option1", "Some useful string option", false);
const args = parser.parse().unwrap();

print(args.option, args.option1);
```

CLI Input / Output:

```
$ dictu argparse.du --option "Hello"
hello
nil
```

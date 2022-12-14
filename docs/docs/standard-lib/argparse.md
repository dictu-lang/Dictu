---
layout: default
title: Argparse
nav_order: 19
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

```cs
from Argparse import Parse;
```

### New Parser(string, string, string)

To create a new parser instance, call the `Parser` class with the 3 required string arguments; name, description, and user provided usage.

```cs
var parser = Parser("prog_name", "Program to do all the things", "");
```

### Parse.addString(string, string, bool, string -> optional)

To add a new string argument, call the method below with at least the 3 required arguments; name, description, and boolean indicating the flag is required. A fourth argument can be passed to give the parser a custom name for the given flag.

```cs
parser.addString("-b", "birth month", false, "month");
```

### Parser.addNumber(string, string, bool, string -> optional)

To add a new number argument, call the method below with at least the 3 required arguments; name, description, and boolean indicating the flag is required. A fourth argument can be passed to give the parser a custom name for the given flag.

```cs
parser.addNumber("--port", "server port", false);
```

### Parser.addBool(string, string, bool, string -> optional)

To add a new bool argument, call the method below with at least the 3 required arguments; name, description, and boolean indicating the flag is required. A fourth argument can be passed to give the parser a custom name for the given flag.

```cs
parser.addBool("-s", "run securely", true, "secure");
```

### Parser.addList(string, string, bool, string -> optional)

To add a new list argument, call the method below with at least the 3 required arguments; name, description, and boolean indicating the flag is required. A fourth argument can be passed to give the parser a custom name for the given flag.

```cs
parser.addList("-u", "active users", true, "users");
```

### Parser.parse()

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

The value of type `Args` will be instantiated with fields matching the configured flags or the given metavar names. Below is an example using the list argument example from above.

```cs
print("Users: {}".format(args.users));
```

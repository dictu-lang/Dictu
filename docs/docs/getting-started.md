---
layout: default
title: Getting started
nav_order: 2
---

# Getting Started
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---
## Hello World!

We've already learned how to enter the REPL and run code there. Now, let's create a Dictu file and execute it. Open your favourite code editor, and create a new file with a `.du` extension. If you're new to programming or aren't sure which editor to use, [Visual Studio Code](https://code.visualstudio.com/) (with the [Dictu extension](https://github.com/dictu-lang/DictuVSC)), or [Sublime Text](https://www.sublimetext.com/) are both fantastic (and free) choices.

We're going to call our file `hello-world.du`:

```cs
print("Hello, World!");
```

That's all we'll put in it for now. Let's execute our file. It's just like entering the REPL, except we add the path to our file after `./dictu`.

```bash
$ ./dictu hello-world.du
Hello, World!
```

As you see, `Hello, World!` is printed to the screen. You have just created your first Dictu programme, now onto bigger and better things!

The `hello-world.du` file can be called directly from the shell. Two things are required:

* Make the file executable: `chmod +x hello-world.du`
* Add a shebang line to the top of the script: `#!/usr/bin/env dictu`

## REPL

The REPL is a language shell, allowing you to run code on the fly via your terminal. This is very useful for debugging, or small checks you may want to perform. When you enter the REPL, there is a magic variable `_` which stores the last returned value (`nil` excluded).

```
Dictu Version: {{ site.version }}
>>> var someNumber = 10;
>>> 5 + someNumber;
15
>>> _;
15
```

## Comments

It's always important to comment your code! Comments are used to explain what your code does. Dictu's syntax will be familiar to any programmer.

```cs
// This is a single line comment

/*
This is a
multiline comment
*/
```

{: .no_toc }
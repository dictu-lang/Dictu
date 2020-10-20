---
layout: default
title: Welcome
nav_order: 1
permalink: /
---

# Dictu v{{ site.version }}
{: .fs-9 }

Dictu is a simple, dynamically typed programming language.
{: .fs-6 .fw-300 }

[Install in 30 Seconds](#installing-dictu){: .btn .btn-primary .fs-5 .mb-4 .mb-md-0 .mr-2 } [View on GitHub](https://github.com/dictu-lang/Dictu){: .btn .fs-5 .mb-4 .mb-md-0 }

```cs
import HTTP;
import JSON;

var data = HTTP.get("https://api.coindesk.com/v1/bpi/currentprice.json");
data = JSON.parse( data["content"] );
print( "$" + data["bpi"]["USD"]["rate"] + " per BTC" ); // $10,577.70 per BTC
```

Dictu is a very small, simple, and dynamically typed programming language inspired by a [book by Robert Nystrom](http://www.craftinginterpreters.com/contents.html). Dictu builds upon the concepts within the book and adds more features to the language.

Dictu means simplistic in Latin. This is the aim of the language: to be as simplistic, organized, and logical as humanly possible.

{: .fs-6 .fw-300 }

---

## Installing Dictu
All it takes is a couple of lines! Fire up a terminal and copy the following, one by one (without the $).

### CMake

```bash
$ git clone https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ cmake -DCMAKE_BUILD_TYPE=Release -B ./build 
$ cmake --build ./build
$ ./build/Dictu
```

### Makefile

```bash
$ git clone https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ make dictu
```

Let's make sure that everything went as planned by entering the Dictu REPL.

```bash
$ ./dictu
```

You should be able to run Dictu code here! REPL stands for “read evaluate print loop” and it allows you to receive instantaneous feedback about your code.

```
Dictu Version: {{ site.version }}
>>> print("Hello, World!");
```

Use `Control + C` to exit the REPL when you're finished.

## Contributing
We want to make the Dictu community as friendly and welcoming as possible, so [pull requests](https://github.com/dictu-lang/Dictu/pulls) are encoraged. Any contribution, from the smallest typo in the documentation to a new feature in the language itself, is greatly appreciated!

## License
Dictu is under the [MIT license](https://github.com/dictu-lang/Dictu/blob/master/LICENSE).
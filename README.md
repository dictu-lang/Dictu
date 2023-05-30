# Dictu

*What is Dictu?*

Dictu is a high-level dynamically typed, multi-paradigm, interpreted programming language. Dictu has a very familiar
C-style syntax along with taking inspiration from the family of languages surrounding it, such as Python and JavaScript. 

*What does Dictu mean?*

Dictu means `simplistic` in Latin.

### Dictu documentation
Documentation for Dictu can be found [here](https://dictu-lang.com/)

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/ab84059049bd4ba7b7b8c1fcfaac4ea5)](https://app.codacy.com/manual/jasonhall96686/Dictu?utm_source=github.com&utm_medium=referral&utm_content=Jason2605/Dictu&utm_campaign=Badge_Grade_Dashboard)
[![CI](https://github.com/Jason2605/Dictu/workflows/CI/badge.svg)](https://github.com/Jason2605/Dictu/actions)

## Example programs
```cs
import System;

const guess = 10;

while {
    const userInput = input("Input your guess: ").toNumber().unwrap();
    if (userInput == guess) {
        print("Well done!");
        break;
    } else if (userInput < guess) {
        print("Too low!");
    } else {
        print("Too high!");
    }

    System.sleep(1);
}
```

```cs
def fibonacci(num) {
    if (num < 2) {
        return num;
    }

    return fibonacci(num - 2) + fibonacci(num - 1);
}

print(fibonacci(10));
```

More [here.](https://github.com/Jason2605/Dictu/tree/develop/examples)

## Running Dictu

Dictu requires that you have CMake installed and it is at least version 3.16.3.

### Using CMake (at least version 3.16.3 or greater)

```bash
$ git clone -b master https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ cmake -DCMAKE_BUILD_TYPE=Release -B ./build
$ cmake --build ./build # on Windows add "--config Release" here to get a Release build
$ ./dictu # on Windows the executable is ".\Release\dictu.exe"
```

### Using CMake presets (version 3.21.0 or greater)

```bash
$ git clone -b master https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ cmake --preset release
$ cmake --build --preset release
$ ./dictu # on Windows the executable is ".\Release\dictu.exe"
```

#### Compiling without HTTP

The HTTP class within Dictu requires [cURL](https://curl.haxx.se/) to be installed when building the interpreter. If you wish to
build Dictu without cURL, and in turn the HTTP class, build with the `DISABLE_HTTP` flag.

##### Without CMake presets (at least version 3.16.3 or greater)

```bash
$ git clone -b master https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ cmake -DCMAKE_BUILD_TYPE=Release -DDISABLE_HTTP=1 -B ./build 
$ cmake --build ./build # on Windows add "--config Release" here to get a Release build
$ ./dictu # on Windows the executable is ".\Release\dictu.exe"
```

##### CMake presets (version 3.21.0 or greater)

```bash
$ git clone -b master https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ cmake --preset release-nohttp
$ cmake --build --preset release
$ ./dictu # on Windows add "--config Release" here to get a Release build
```

### Docker Installation

Refer to [Dictu Docker](https://github.com/dictu-lang/Dictu/blob/develop/Docker/README.md)

### FreeBSD Installation

For a full installation, make sure `curl` is installed. It can be installed from the commands below:

```bash
$ pkg install -y curl linenoise-ng
```

The following variables need to be set/available to run `cmake` successfully.

For Bourne compatible shells...

```bash
export CPATH=/usr/local/include
export LIBRARY_PATH=/usr/local/lib
export LD_LIBRARY_PATH=/usr/local/lib
```

```bash
$ git clone -b master https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ cmake -DCMAKE_BUILD_TYPE=Release -B ./build 
$ cmake --build ./build
$ ./dictu
```

## Extensions

Dictu has a Visual Studio Code extension [here](https://marketplace.visualstudio.com/items?itemName=Dictu.dictuvsc) with the implementation located
in the [DictuVSC repo](https://github.com/dictu-lang/DictuVSC).

## Credits

This language was initially based on the very good [craftinginterpreters book](http://www.craftinginterpreters.com/contents.html), along with inspiration from [Wren](https://github.com/wren-lang/wren).

<p>This project is supported by:</p>
<p>
  <a href="https://m.do.co/c/02bd923f5cda">
    <img src="https://opensource.nyc3.cdn.digitaloceanspaces.com/attribution/assets/SVG/DO_Logo_horizontal_blue.svg" width="201px">
  </a>
</p>

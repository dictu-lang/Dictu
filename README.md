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
```js
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

```js
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

### CMake
```bash
$ git clone -b master https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ cmake -DCMAKE_BUILD_TYPE=Release -B ./build 
$ cmake --build ./build
$ ./dictu
```

#### Compiling without HTTP

The HTTP class within Dictu requires [cURL](https://curl.haxx.se/) to be installed when building the interpreter. If you wish to
build Dictu without cURL, and in turn the HTTP class, build with the `DISABLE_HTTP` flag.

```bash
$ git clone -b master https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ cmake -DCMAKE_BUILD_TYPE=Release -DDISABLE_HTTP=1 -B ./build 
$ cmake --build ./build
$ ./dictu
```

#### Compiling without linenoise
[Linenoise](https://github.com/antirez/linenoise) is used within Dictu to enhance the REPL, however it does not build on windows
systems so a simpler REPL solution is used.

```bash
$ git clone -b master https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ cmake -DCMAKE_BUILD_TYPE=Release -DDISABLE_LINENOISE=1 -B ./build 
$ cmake --build ./build
$ ./build/Dictu
```

### Docker Installation

Refer to [Dictu Docker](https://github.com/dictu-lang/Dictu/blob/develop/Docker/README.md)

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

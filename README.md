# Dictu

*What is Dictu?*  
Dictu is a very simple dynamically typed programming language 
built upon the [craftinginterpreters tutorial](http://www.craftinginterpreters.com/contents.html).

*What does Dictu mean?*  
Dictu means `simplistic` in Latin.

### Dictu documentation
Documentation for Dictu can be found [here](https://dictu-lang.com/)

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/ab84059049bd4ba7b7b8c1fcfaac4ea5)](https://app.codacy.com/manual/jasonhall96686/Dictu?utm_source=github.com&utm_medium=referral&utm_content=Jason2605/Dictu&utm_campaign=Badge_Grade_Dashboard)
[![CI](https://github.com/Jason2605/Dictu/workflows/CI/badge.svg)](https://github.com/Jason2605/Dictu/actions)

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


## Example program
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

More [here.](https://github.com/Jason2605/Dictu/tree/develop/examples)

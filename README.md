<p align="center">
    <a href="https://dictu-lang.com">
        <img src="docs/assets/images/dictu-logo/dictu-light-png-8x.png" alt="Dictu logo" width="560px">
    </a>
</p>

--------------------------------------------------------------------------------

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/ab84059049bd4ba7b7b8c1fcfaac4ea5)](https://app.codacy.com/manual/jasonhall96686/Dictu?utm_source=github.com&utm_medium=referral&utm_content=Jason2605/Dictu&utm_campaign=Badge_Grade_Dashboard)
[![CI](https://github.com/Jason2605/Dictu/workflows/CI/badge.svg)](https://github.com/Jason2605/Dictu/actions)

*What is Dictu?*  
Dictu is a very simple dynamically typed programming language 
built upon the [craftinginterpreters tutorial](http://www.craftinginterpreters.com/contents.html).

*What does Dictu mean?*  
Dictu means `simplistic` in Latin.

### Dictu documentation
Documentation for Dictu can be found [here](https://dictu-lang.com/)

## Running Dictu
Dictu currently has two options when building, there is a CMakeLists file included so the build files can be generated with
CMake or there is an included makefile for users that are more familiar with that.

### CMake
```bash
$ git clone https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ cmake -DCMAKE_BUILD_TYPE=Release -B ./build 
$ cmake --build ./build
$ ./build/Dictu
```

#### Compiling without HTTP

The HTTP class within Dictu requires [cURL](https://curl.haxx.se/) to be installed when building the interpreter. If you wish to
build Dictu without cURL, and in turn the HTTP class, build with the `DISABLE_HTTP` flag.

```bash
$ git clone https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ cmake -DCMAKE_BUILD_TYPE=Release -DDISABLE_HTTP=1 -B ./build 
$ cmake --build ./build
$ ./build/Dictu
```

#### Compiling without linenoise
[Linenoise](https://github.com/antirez/linenoise) is used within Dictu to enhance the REPL, however it does not build on windows
systems so a simpler REPL solution is used.

```bash
$ git clone https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ cmake -DCMAKE_BUILD_TYPE=Release -DDISABLE_LINENOISE=1 -B ./build 
$ cmake --build ./build
$ ./build/Dictu
```

### Makefile
```bash
$ git clone https://github.com/dictu-lang/Dictu.git
$ cd Dictu
$ make dictu
$ ./dictu examples/guessingGame.du
```

#### Compiling without HTTP

The HTTP class within Dictu requires [cURL](https://curl.haxx.se/) to be installed when building the interpreter. If you wish to
build Dictu without cURL, and in turn the HTTP class, build with the `DISABLE_HTTP` flag.

```bash
$ git clone https://github.com/Jason2605/Dictu.git
$ cd Dictu
$ make dictu DISABLE_HTTP=1
$ ./dictu examples/guessingGame.du
```

## Example program
```js
var userInput;
var guess = 10;

while {
    userInput = input("Input your guess: ").toNumber();
    print(userInput);
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

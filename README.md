# Dictu

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/ab84059049bd4ba7b7b8c1fcfaac4ea5)](https://app.codacy.com/manual/jasonhall96686/Dictu?utm_source=github.com&utm_medium=referral&utm_content=Jason2605/Dictu&utm_campaign=Badge_Grade_Dashboard)

*What is Dictu?*  
Dictu is a very simple dynamically typed programming language 
built upon the [craftinginterpreters tutorial](http://www.craftinginterpreters.com/contents.html).

*What does Dictu mean?*  
Dictu means `simplistic` in Latin.

### Dictu documentation
Documentation for Dictu can be found [here](https://jason2605.github.io/Dictu/)

## Running Dictu
```bash
$ git clone https://github.com/Jason2605/Dictu.git
$ cd Dictu
$ make dictu
$ ./dictu examples/guessingGame.du
```

### Compiling without HTTP

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

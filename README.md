# Dictu

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

## Example program
```js
def guessingGame() {
    var userInput;
    var guess = 10;

    while {
        userInput = number(input("Input your guess: "));
        print(userInput);
        if (userInput == guess) {
            print("Well done!");
            return;
        } else if (userInput < guess) {
            print("Too low!");
        } else {
            print("Too high!");
        }

        sleep(1);
    }
}

guessingGame();
```

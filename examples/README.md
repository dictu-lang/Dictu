# Example Dictu programs
## Classes
```js
// Setup a new trait
trait PrintNameTrait {
    printName() {
        print(this.name);
    }
}

// Setup a new class
class Person {
    // Use the trait and inherit the methods
    use PrintNameTrait;

    // Class constructor
    init(name, age) {
        this.name = name;
        this.age = age;
    }

    // Define a new method
    printAge() {
        print("{} is {} years old!".format(this.name, this.age));
    }
}

// Setup a new class
class Car {
    // Use the trait and inherit the methods
    use PrintNameTrait;

    // Class constructor
    init(name, model) {
        this.name = name;
        this.model = model;
    }

    // Define a new method
    printModel() {
        print("{} is a {} model".format(this.name, this.model));
    }
}

// Instantiate a new Person object
var jason = Person("Jason", 500);

// Call the method defined in the trait
jason.printName();

// Call the method defined in the class
jason.printAge();

// Instantiate a new Car object
var cayman = Car("Cayman", "Porsche");

// Call the method defined in the trait
cayman.printName();

// Call the method defined in the class
cayman.printModel();
```
[See here](https://github.com/Jason2605/Dictu/blob/develop/examples/classes.du)

## Factorial
```js
var amount = input("Enter a number: ").toNumber();
var num = 1;

if (amount > 0) {
    for (var i = 1; i < amount + 1; ++i) {
        num *= i;
    }

    print("Factorial of {} is {}".format(amount, num));
} else if (amount == 0) {
    print("The factorial of 0 is 1");
} else {
    print("Can't factorial a negative!");
}
```

[See here](https://github.com/Jason2605/Dictu/blob/develop/examples/factorial.du)

## FizzBuzz

```js
for (var i = 1; i < 101; ++i) {
    if (i % 15 == 0) {
        print("FizzBuzz");
    } else if (i % 3 == 0) {
        print("Fizz");
    } else if (i % 5 == 0) {
        print("Buzz");
    }
}
```

[See here](https://github.com/Jason2605/Dictu/blob/develop/examples/fizzBuzz.du)

## Guessing Game

```js
var guess = 10;

while {
    var userInput = input("Input your guess: ").toNumber();
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

[See here](https://github.com/Jason2605/Dictu/blob/develop/examples/guessingGame.du)

## Recursive Fibonacci

```js
def fibonacci(num) {
    if (num < 2) {
        return num;
    }

    return fibonacci(num - 2) + fibonacci(num - 1);
}

print(fibonacci(10));
```

[See here](https://github.com/Jason2605/Dictu/blob/develop/examples/recursiveFib.du)


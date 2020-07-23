# Design Patterns
This directory shows how some of the commonly used OOP design patterns would be 
implemented in Dictu. The code for the majority of the patterns has come from
the [Refactoring Guru](https://refactoring.guru/) website.

## Builder
```js
class Builder {
    init() {
        this.reset();
    }

    reset() {
        this.product = Product();
    }

    producePartA() {
        this.product.parts.push("PartA");
    }

    producePartB() {
        this.product.parts.push("PartB");
    }

    producePartC() {
        this.product.parts.push("PartC");
    }

    getProduct() {
        var result = this.product;
        this.reset();

        return result;
    }
}

class Product {
    init() {
        this.parts = [];
    }

    listParts() {
        print("Product parts: {}".format(this.parts.join()));
    }
}

/**
 * Director class is an optional class in the builder pattern
 *
 * It is useful if we wish to build products in a specific order
 */
class Director {
    init(builder) {
        this.builder = builder;
    }

    minimumProduct() {
        this.builder.producePartA();
    }

    fullProduct() {
        this.builder.producePartA();
        this.builder.producePartB();
        this.builder.producePartC();
    }
}

/**
 * We can ignore the director if we wish to build products manually
 */
var builder = Builder();

builder.producePartA();
builder.producePartB();

var product = builder.getProduct();
product.listParts();

builder.producePartA();
builder.producePartB();
builder.producePartC();

product = builder.getProduct();
product.listParts();

/**
 * We can introduce a "director" to create
 * "known" products for us.
 */
var director = Director(builder);

director.minimumProduct();
builder.getProduct().listParts();

director.fullProduct();
builder.getProduct().listParts();
```

## Chain of responsibility
```js
abstract class BaseHandler {
    init() {
        this.nextHandler = nil;
    }

    setNext(handler) {
        this.nextHandler = handler;
        return handler;
    }

    handle(request) {
        if (this.nextHandler) {
            return this.nextHandler.handle(request);
        }
    }
}

class MonkeyHandler < BaseHandler {
    handle(request) {
        if (request == "Banana") {
            return "Monkey: I'll eat the banana";
        }

        return super.handle(request);
    }
}

class SquirrelHandler < BaseHandler {
    handle(request) {
        if (request == "Nut") {
            return "Squirrel: I'll eat the nut";
        }

        return super.handle(request);
    }
}

class DogHandler < BaseHandler {
    handle(request) {
        if (request == "Meatball") {
            return "Dog: I'll eat the meatball";
        }

        return super.handle(request);
    }
}

def businessLogic(handler) {
    var food = ["Nut", "Banana", "Coffee"];

    for (var i = 0; i < food.len(); ++i) {
        print("Who wants a: {}".format(food[i]));

        var response = handler.handle(food[i]);

        if (response) {
            print(response);
        } else {
            print("No one wanted a: {}".format(food[i]));
        }
    }
}

var monkey = MonkeyHandler();
var squirrel = SquirrelHandler();
var dog = DogHandler();

monkey.setNext(squirrel).setNext(dog);

businessLogic(monkey);
```

## Observer
```js
class Publisher {
    init() {
        this.observers = [];
    }

    /**
     * Attach an observer object to this publisher
     *
     * @param observer
     */
    attach(observer) {
        this.observers.push(observer);
    }

    /**
     * Remove an observer from this publisher
     *
     * @param observer
     */
    remove(observer) {
        this.observers.remove(observer);
    }

    /**
     * Notify all subscribed observers
     */
    notify() {
        for (var i = 0; i < this.observers.len(); ++i) {
            this.observers[i].update();
        }
    }

    /**
     * Main business logic
     */
    main() {
        print("Super important logic!!");

        this.notify();
    }
}

class ObserverA {
    /**
     * Function ran when the publisher notifies us
     */
    update() {
        print("ObserverA has been notified!");
    }
}

class ObserverB {
    /**
     * Function ran when the publisher notifies us
     */
    update() {
        print("ObserverB has been notified!");
    }
}

var pub = Publisher();

var observerA = ObserverA();
var observerB = ObserverB();

pub.attach(observerA);
pub.attach(observerB);

pub.main();
```
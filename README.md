# Dictu

*What is Dictu?*  
Dictu is a very simple dynamically typed programming language 
built upon the [craftinginterpreters tutorial](http://www.craftinginterpreters.com/contents.html).

*What does Dictu mean?*  
Dictu means `simplistic` in Latin.

### Dictu language examples

*Variable declaration:*
```
var someNumber = 10;
var someString = "hello";
var someOtherString = 'hello';
var someBoolean = true;
var someFalseBoolean = false;
var someNilValue = nil;
```
*Comments*
```
//This is a single line comment

/*
this
is
a
multiline
comment
*/
```
*If statements*
``` 
if (false or true) {
    var variable = "hello";
    print variable;
}

//Single line if statement body does not require braces
if (true)
    print "hello";
```
*Conditional expressions*
```
var someVar = 10;
var someTestVar = 11;

if (someVar == 10 and someTestVar == 11)
    print "True!";
    
if (someVar == 9 or someTestVar == 11)
    print "True!";
    
if (someVar >= 9)
    print "True!";
    
if (someVar <= 9)
    print "False!";
    
if (someVar > 9)
    print "True!";
    
if (someVar < 9)
    print "False!";
```

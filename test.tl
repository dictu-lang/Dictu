def test() {
    print 1;
}

def hello() {
  var test = 10;
  var test1 = 11;

  if (test == 10) {
      print "hello";
      print "what";
      print "?????";
      print (1 and true);
  }
  print "ok then";

}

class testing {
  init() {
    this.test1 = 10;
    //return this;
  }

  //static hello() {
  //  print "this";
  //}

  testing() {
    print "hi";
  }

  test() {
    print this.test1;
    print 2;
  }
}

test();

hello();

var obj = testing();
//print obj;
//obj.testing();


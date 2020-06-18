import time
start = time.clock()

for _ in range(10000):
    x = "{} {}".format("test", "test")

print(time.clock() - start)
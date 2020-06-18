import time

start = time.clock()

for i in range(10000):
    x = "aaaaa".replace("a", "e")

print(time.clock() - start)

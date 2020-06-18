import time
start = time.clock()

for _ in range(10000):
    x = "dictu is great!".upper()

print(time.clock() - start)
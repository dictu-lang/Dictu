import time
start = time.clock()

for _ in range(10000):
    x = "Dictu is great!".split(" ")

print(time.clock() - start)
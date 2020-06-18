import time
start = time.clock()

x = {"dictu": "is great!"}

for _ in range(10000):
    "dictu" in x

print(time.clock() - start)
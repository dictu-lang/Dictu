import time
start = time.clock()
x = ["Dictu is great!"]

for _ in range(10000):
    "Dictu is great!" in x

print(time.clock() - start)
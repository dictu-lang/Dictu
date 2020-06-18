import time
x = ["Dictu is great!"] * 10000

start = time.clock()

for _ in range(10000):
    x.pop()

print(time.clock() - start)
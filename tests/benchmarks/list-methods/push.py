import time
start = time.clock()
x = []

for _ in range(10000):
    x.append("Dictu is great!")

print(time.clock() - start)
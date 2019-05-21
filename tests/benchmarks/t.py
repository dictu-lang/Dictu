import time

x = time.clock()

d = {}

for i in range(1, 1000001):
    d[str(i)] = i

print(time.clock() - x)
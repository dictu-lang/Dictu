import time
import copy
start = time.clock()
x = {"Dictu": "is great!"}

for _ in range(10000):
    copy.deepcopy(x)

print(time.clock() - start)
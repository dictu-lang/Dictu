import time
import copy
start = time.perf_counter()
x = {"Dictu": "is great!"}

for _ in range(10000):
    copy.deepcopy(x)

print(time.perf_counter() - start)
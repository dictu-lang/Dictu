import time
start = time.perf_counter()

for _ in range(10000):
    x = "DICTU IS GREAT!".lower()

print(time.perf_counter() - start)
import time
start = time.perf_counter()

for _ in range(10000):
    "test   ".strip()

print(time.perf_counter() - start)
import time
start = time.perf_counter()

for _ in range(10000):
    x = "dictu is great!".upper()

print(time.perf_counter() - start)
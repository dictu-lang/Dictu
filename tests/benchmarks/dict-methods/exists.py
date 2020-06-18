import time
start = time.perf_counter()

x = {"dictu": "is great!"}

for _ in range(10000):
    "dictu" in x

print(time.perf_counter() - start)
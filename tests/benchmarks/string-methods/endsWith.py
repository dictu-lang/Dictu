import time
start = time.perf_counter()

for _ in range(10000):
    x = "Dictu is great!".endswith("great!")

print(time.perf_counter() - start)
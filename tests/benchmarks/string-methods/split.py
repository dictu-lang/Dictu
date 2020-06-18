import time
start = time.perf_counter()

for _ in range(10000):
    x = "Dictu is great!".split(" ")

print(time.perf_counter() - start)
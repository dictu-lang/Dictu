import time
start = time.perf_counter()

for _ in range(10000):
    ", ".join(str(x) for x in [1, 2, 3, 4, 5, 6, 7, 8, 9, 10])

print(time.perf_counter() - start)
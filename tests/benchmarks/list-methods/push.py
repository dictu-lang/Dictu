import time
start = time.perf_counter()
x = []

for _ in range(10000):
    x.append("Dictu is great!")

print(time.perf_counter() - start)
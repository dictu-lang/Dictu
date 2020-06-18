import time
x = ["Dictu is great!"] * 10000

start = time.perf_counter()

for _ in range(10000):
    x.pop()

print(time.perf_counter() - start)
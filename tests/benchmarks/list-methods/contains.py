import time
start = time.perf_counter()
x = ["Dictu is great!"]

for _ in range(10000):
    "Dictu is great!" in x

print(time.perf_counter() - start)
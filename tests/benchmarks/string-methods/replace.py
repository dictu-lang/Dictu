import time

start = time.perf_counter()

for i in range(10000):
    x = "aaaaa".replace("a", "e")

print(time.perf_counter() - start)

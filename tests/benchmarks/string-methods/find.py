import time

start = time.perf_counter()

for i in range(10000):
    x = "Dictu is great!".find("is")

print(time.perf_counter() - start)

import time
x = {i: "Dictu is great!" for i in range(10000)}

start = time.perf_counter()

for i in range(10000):
    x.pop(i)

print(time.perf_counter() - start)
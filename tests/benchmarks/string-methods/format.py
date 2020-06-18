import time
start = time.perf_counter()

for _ in range(10000):
    x = "{} {}".format("test", "test")

print(time.perf_counter() - start)
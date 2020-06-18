import time
start = time.clock()

for _ in range(10000):
    x = "Dictu is great!".startswith("Dictu")

print(time.clock() - start)
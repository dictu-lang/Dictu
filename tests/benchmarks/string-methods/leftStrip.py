import time
start = time.clock()

for _ in range(10000):
    "   test".strip()

print(time.clock() - start)
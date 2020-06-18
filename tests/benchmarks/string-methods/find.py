import time

start = time.clock()

for i in range(10000):
    x = "Dictu is great!".find("is")

print(time.clock() - start)

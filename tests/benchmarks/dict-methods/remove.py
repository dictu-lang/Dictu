import time
x = {str(i): "Dictu is great!" for i in range(10000)}

start = time.clock()

for i in range(10000):
    x.pop(str(i))

print(time.clock() - start)
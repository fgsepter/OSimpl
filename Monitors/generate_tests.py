import random 

for i in range(10):
    with open("disk.in"+str(i), 'w') as file:
        for j in range(10000):
            file.write(str(random.randint(1,1000)) +"\n")
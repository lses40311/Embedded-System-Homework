import csv
import random
import math

N = 10000
f = open('test.txt', 'w')
# writer = csv.writer(f, delimiter=',')
for i in range(0,N):
	f.write(str(int(random.uniform(0,100)))+ "\n")
f.close()	

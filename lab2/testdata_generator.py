import csv
import random

N = 10000
f = open('test.txt', 'w')
writer = csv.writer(f, delimiter=',')
for i in range(0,N):
	writer.writerow([random.uniform(0,100), random.randint(0,100)])
f.close()	

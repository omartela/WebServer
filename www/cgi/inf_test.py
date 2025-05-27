import sys
import time

while True:
     sys.stdout.write("A")
     with open("test123.txt", "a") as f:
         f.write("A")
     time.sleep(1)
     break

#while True:
    #sys.stdout.write("A")
    #with open("test123.txt", "a") as f:
        #f.write("A")
    #time.sleep(1)

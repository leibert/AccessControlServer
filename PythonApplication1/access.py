#access server
import os



print "list"
dirs = os.listdir("../IDS/") # returns list

d={}


for file in dirs:
    if file.endswith(".ID"):
        print file
        with open("../IDS/"+file, 'r') as file:
            # with open('housestates.dat', 'r') as file:
            for line in file:
                print line
        






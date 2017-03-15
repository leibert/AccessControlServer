#!/usr/bin/python

#access server
import os
import sys
import argparse
import cgitb, cgi
import threading
import requests

cgitb.enable()


print "Content-type: text/html\n\n"
# print "HELLO FROM IOS.py"
cgiinput = cgi.FieldStorage()


cardrolllocation="/var/www/html/espserve/CMDCTRL/cardroll.dat"
IDfilelocation="/opt/access/IDS/"



class sendAUDIOcmd (threading.Thread):
    def __init__(self, path):
        threading.Thread.__init__(self)
        self.path = path

    def run(self):
        print "Conacting media center"

        #data = urllib.urlencode(self.path)

        #url = "HOUSEMEDIADIR:8080/play"

        print self.path

        #conn = httplib.HTTPConnection(url)
        #conn.request('POST', '/', self.path)
        #resp = conn.getresponse()
        #content = resp.read()
        #print content

        #r = requests.get('https://api.github.com/user', auth=('user', 'pass'))
        r= requests.post('http://HOUSEMEDIADIR:8080/play', data=self.path)
        print r.status_code
        print r.text
        #self.exit()


        print "Exiting " + self.name



def sendAUDIO(audiopath):
    print "ThreadESPsend"
#     thread.start_new_threa/d(sendESPcommandworker, (IP,))
    thread=sendAUDIOcmd(audiopath)
    print thread
    thread.start()







def updatecardroll():
    print "Updating Card roll"
    dirs = os.listdir(IDfilelocation) # returns list

    cardroll={}


    for file in dirs:
        if file.endswith(".ID"):
            ID = file[0:file.find(".ID")]
            #print ID
            with open(IDfilelocation+file, 'r') as file:
                #print file.readline()
                cardroll[ID]=file.readline().rstrip();

                
    print cardroll
    if len(cardroll) > 0:
        writecardroll(cardroll)
        


def writecardroll(cardroll):
    try:
        f = open(cardrolllocation, 'w')

        f.seek(0)

        for key, value in cardroll.iteritems():
            f.write(key+","+value)
            f.write('\n')

        f.truncate()
        f.close()
    except:
        print "error writing"


def runIDcommands(ID):
    print "run ID command"
    print ID
    try:

        IDfile=IDfilelocation+str(ID)+".ID" 
        print IDfile

        with open(IDfile, 'r') as file:
            #print file
            for line in file:
            
                if "AMD" in line:
                    print "There is a music command in here"
                    musicpath = line[line.find("AMD:")+4:]
                    musicpath = "LCL" + musicpath
                    print musicpath
                    sendAUDIO(musicpath)
        
    except:
        print "error opening ID card dat file";
            #print file.readline()
            #cardroll[ID]=file.readline().rstrip();



#updatecardroll()
#runIDcommands(2080060114)




if "mode" in cgiinput:  # mode/funciton selection
    print "mode exists" + cgiinput.getvalue("mode")
    if cgiinput.getvalue("mode") == 'doorOPENtx':
        print "doorOPENtx mode"
        runIDcommands(cgiinput.getvalue("ID"))










parser = argparse.ArgumentParser()
#
parser.add_argument('--updateroll', action='store_true', help='update card roll')
# # parser.add_argument('--weather', action='store_true', help='update weather')
# # parser.add_argument('--astronomy', action='store_true', help='update astronomy')
# # parser.add_argument('--state', action='store', help='update a house state key=value')
# # # parser.add_argument('--weather', action='store_true', help='update weather') Do something for coffee...action bots? pass in a CMD
# parser.add_argument('--all', action='store_true', help='update all')
#

args = parser.parse_args()
# # print args
#
if args.updateroll:
     print "updateroll"
     updatecardroll()


#sendAUDIO("AMD:TESTMUSIC/test.mp3")
print "bottom of file"
#P2000 ONTVANGEN
#
# https://nl.oneguyoneblog.com/2016/08/09/p2000-ontvangen-decoderen-raspberry-pi/
#
#
#	Important: adjust the line that defines the radio
#	here: sdrplay-radio or airspy-radio
#
import time
import sys
import subprocess
import os
import re
from datetime import datetime
from dateutil import tz
from termcolor import colored
 
groupidold = ""
 
def curtime():
    return time.strftime("%H:%M:%S %Y-%m-%d")
 
with open('error.txt','a') as file:
    file.write(('#' * 20) + '\n' + curtime() + '\n')

#
#	adjust the radio
multimon_ng = subprocess.Popen("airspy-radio -f 169650000 -g 90 | multimon-ng -a FLEX -t raw -",
                               stdout=subprocess.PIPE,
                               stderr=open('error.txt','a'),
                               shell=True)
 
try:
    while True:
        line = multimon_ng.stdout.readline()
        multimon_ng.poll()
        if line.__contains__("ALN"):
 
		if line.startswith('FLEX'):
 
			flex = line[0:5]
			timestamp = line[6:25]
			melding = line[58:]
			groupid = line[35:41]
			capcode = line[43:52]
			regex_prio1 = "^A\s?1|\s?A\s?1|PRIO\s?1|^P\s?1"
			regex_prio2 = "^A\s?2|\s?A\s?2|PRIO\s?2|^P\s?2"
			regex_prio3 = "^B\s?1|^B\s?2|^B\s?3|PRIO\s?3|^P\s?3|PRIO\s?4|^P\s?4"
 
		if re.search(regex_prio1, melding, re.IGNORECASE):
			priokleur = 'red'
		
		elif re.search(regex_prio2, melding, re.IGNORECASE):
			priokleur = 'yellow'
 
		elif re.search(regex_prio3, melding, re.IGNORECASE):
			priokleur = 'green'
 
		else:
			priokleur = 'magenta'

		if groupid == groupidold:
			print colored(capcode, 'white'),
		else:
 
			utc = datetime.strptime(timestamp, '%Y-%m-%d %H:%M:%S')
			utc = utc.replace(tzinfo=tz.tzutc())
			local = utc.astimezone(tz.tzlocal())
			local = local.strftime("%d-%m-%Y %H:%M:%S")
			
			print ' '
			print colored(local,'blue', attrs=['bold']), colored(melding, priokleur,  attrs=['bold']),
			print '                  ',
			print colored(capcode, 'white'),
 
			groupidold = groupid
except KeyboardInterrupt:
    os.kill(multimon_ng.pid, 9)


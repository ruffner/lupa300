# ajile filer v1.0
# Matt Ruffner Dec 2018
#
# since ajile stores the -full- project working directory
# this script changes the working paths of all parts of an
# ajile project to the currect folder on the system
#
# for example if you copy an ajile project to a new computer
# you should first run this to update the ajile xml file to reflect
# the full working path as it is on the new system
#
# todo: optimize

import xml.etree.ElementTree as ET
import os

cwd=os.getcwd()
filename=cwd.split('\\')[-1] + '.xml'

proj=ET.parse(filename)
root=proj.getroot()

for ch in root.getiterator():
    if ch.tag == 'workingPath_':
        if ch.text != cwd:
            ch.text = cwd
        print('Changed ajile wd to: ',ch.text,'\n')
        
        
for ch in root[0][4]:
    if ch.tag=='count':
        print('FIXING ',ch.text, ' ', root[0][4].tag,' paths')
    if ch.tag=='item':
        path=ch.getchildren()[1].find('filename_').text
        parts=path.split('/')
        imname='/'+parts[-2]+'/'+parts[-1]
        imname=cwd+imname
        ch.getchildren()[1].find('filename_').text=imname
        print('.',end='')
print('\ndone!\n')


for ch in root[0][5]:
    if ch.tag=='count':
        print('FIXING ',ch.text, ' ', root[0][5].tag,' paths')
    if ch.tag=='item':
        path=ch.getchildren()[1].find('filename_').text
        parts=path.split('/')
        imname='/'+parts[-2]+'/'+parts[-1]
        imname=cwd+imname
        ch.getchildren()[1].find('filename_').text=imname
        print('.',end='')
print('\ndone!')


print('\n\nsaving ajile project file...')
proj.write(filename,method='html')

header = '<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>\n<!DOCTYPE boost_serialization>\n'
with open(filename, 'r') as original: data = original.read()
with open(filename, 'w') as modified: modified.write(header + data)

print('done!')


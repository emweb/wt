#!/usr/bin/python3
#
# Rename to wt3towt4.py?
# Usage: wt3towt4.py directory

import fileinput
import re
import os
import fnmatch
import sys
import shutil

def find(pattern, path):
    result = []
    for root, dirs, files in os.walk(path):
        for name in files:
            if fnmatch.fnmatch(name, pattern):
                result.append(os.path.join(root, name))
    return result

__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))

enumValues = {}
enumRegexes = {}

def readEnumDefinitions():
    #
    # Read enum definitions value into values
    #
    global enumValues
    global enumRegexes

    enums = open(os.path.join(__location__, 'enums.txt'));

    pstart = re.compile("^(\S+)\s+(?:->\s+(\S+)\s+)?{$")
    pvalue = re.compile("^\s+(\S+)(?:\s+->\s+(\S+))?")
    pend = re.compile("^}$")

    for line in enums:
        m = pstart.match(line)
        if m:
            oldenum = m.group(1)
            newenum = m.group(2)
            if not newenum:
                newenum = oldenum
            parts = oldenum.split("::")
            scope = ""
            if len(parts) > 1:
                oldenum = parts[-1]
                scope = "::".join(parts[:-1]) + "::"
    
            enumValues[scope + oldenum] = newenum
            enumRegexes['(?:' + scope + oldenum + '(?!::))'] = 1
    
            for line in enums:
                if pend.match(line):
                    break
                v = pvalue.match(line)
                if v:
                    oldvalue = v.group(1)
                    newvalue = v.group(2)
                    if not newvalue:
                        newvalue = oldvalue
    
                    enumRegexes['(?:' + scope + '(?:' + oldenum + '::' + ')?'
                                + oldvalue + ')'] = 1
                    enumValues[scope + oldvalue] = newenum + '::' + newvalue
                    enumValues[scope + oldenum + '::' + oldvalue] = \
                        newenum + '::' + newvalue

def replaceEnums(infile, outfile):
    def subst(x):
        if x in enumValues.keys():
            return enumValues[x]
        else:
            return x

    pattern = re.compile(r'\b(' + '|'.join(enumRegexes.keys()) + r')\b')
    cpp = re.compile('^\s*\#')
    for line in infile:
        result = line;
        if not cpp.match(line):
            result = pattern.sub(lambda x: subst(x.group()), line)
        outfile.write(result)

def processFile(f):
    print('Converting file: ' + f)
    shutil.copy(f, f + '.bak')
    infile = open(f + '.bak', 'r')
    outfile = open(f, 'w')
    replaceEnums(infile, outfile)

# main
readEnumDefinitions()

files = find ('*', sys.argv[1])
for f in files:
    try:
        if not f.endswith('.txt') and \
           not f.endswith('.js') and \
           not f.endswith('.png') and \
           not f.endswith('.xml') and \
           not f.endswith('.jpg') and \
           not f.endswith('.ico') and \
           not f.endswith('.gif') and \
           not f.endswith('.htm') and \
           not f.endswith('.html') and \
           not f.endswith('.bak') and \
           not f.endswith('.in') and \
           not f.endswith('.css'):
            processFile(f)
    except:
        print ("Error: ", sys.exc_info()[0]) 

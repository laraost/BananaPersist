#!/usr/bin/python3

import re
import sys

if len(sys.argv) != 2:
    print('Usage: convert-to-csv.py INPUTFILE')
    sys.exit()

inputname = sys.argv[1]

# Read the input file and remove anything between # and the next newline
with open(inputname, 'r') as infile:
    data = []
    for line in infile:
        data.append(re.sub(r'#.+\n', '', line))

header_re = re.compile(r'\*\* (?P<names>.+)$')

header_printed = False
for line in data:
    match_header = header_re.match(line)
    if match_header and not header_printed:
        print(match_header[1])
        header_printed = True
    elif not match_header:
        print(line, end = '')

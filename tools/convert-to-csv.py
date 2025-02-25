#!/usr/bin/python3

import re
import sys

if len(sys.argv) != 2:
    print('Usage: convert-to-csv.py INPUTFILE')
    print('')
    print('Assumes that all repetitions in the input file contain the same data')
    sys.exit()

inputname = sys.argv[1]

# Read the input file and remove anything between # and the next newline
with open(inputname, 'r') as infile:
    data = re.sub(r'#.+\n', '', infile.read())

# Regular expression for a repetition block, i.e., anything following a '> rep' until the next '> rep'.
rep_re = re.compile(r'^> rep \d+(?:\.\d+)?(?:\n|\r\n?)(?P<repdata>([^>-].+(?:\n|\r\n?))+)', re.MULTILINE)
# Regular expression for a block of CSV data; one line for the header beginning with '**', one line for the data
csv_re = re.compile(r'(?P<header>\*\* .+\n)(?P<data>.+\n)')
# Regular expression for the csv header
header_re = re.compile(r'\*\* (?P<names>.+)$')

for index, s in enumerate(rep_re.findall(data)):
    csv_matches = csv_re.findall(s[0])
    # Print the csv header for the first rep, so we get a csv header for the total file
    if index == 0:
        for c in csv_matches[:-1]:
            print(c[0].strip('* \n'), end = ', ')
        print(csv_matches[-1][0].strip('* \n'))
    # print the data for each csv block
    for c in csv_matches[:-1]:
        print(c[1].strip(), end = ', ')
    print(csv_matches[-1][1].strip())

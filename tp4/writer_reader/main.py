#!/usr/bin/env python
import os, time


def reader(pipe_in):
	
	string = ""

	string = pipe_in.readline()
	print '%s' % string,



def writer(pipe_out, input_str):

	pipe_out.write(input_str)
	pipe_out.flush()
	time.sleep(1)
	pipe_out.write("end\n")


def main():
	
	if not os.path.exists('pythonToC'):
	    os.mkfifo('pythonToC')

	if not os.path.exists('cToPython'):
	    os.mkfifo('cToPython')


	pythonToC = open('pythonToC','w')
	cToPython = open('cToPython','r')

	reader(cToPython)
	#time.sleep(1)
	writer(pythonToC, "received")
	#writer(pythonToC, "end")
	print 'send'


main()

from PIL import Image
from sys import stdout

def get_mean(values_as_array):
	value = 0 #initialise value as 0
	#add all the numbers up
	for number in values_as_array:
		value += number
	value = value / len(values_as_array) #divide by the number of numbers
	return value #send that back

from os import listdir
files = []
for item in listdir():
	if item.endswith(".png"): 
		files.append(item)
		print(item)

for item in files:
	#load the source image as RGB
	#srcimage = Image.open(input("Enter filename of image: ")).convert('RGB')
	srcimage = Image.open(item).convert('RGB')

	textdata = []
	for Y in range(0,srcimage.height):
		line = ""
		for X in range(0,srcimage.width):
			#Get RGB and compute mean brightness of the RGB components
			R,G,B = srcimage.getpixel((X,Y)) 
			brightness = get_mean([R,G,B])
			if brightness < 128:
				line += "0"
			else:
				line += "1"
		textdata.append(line)
		
	#output the text information properly
	i = 0
	byte = "B"
	print("{")
	for lineindex, y in enumerate(textdata):
		line = "\t" #indent
		for x in y:
			byte += x #append pixel to the byte
			i += 1 #keep track of how many bits that byte has
			if i == 8: #do we have 8 bits? if yes, add that byte to the line and reset byte
				i = 0 #start over at 0
				line += byte + ", "
				byte = "B" #each byte begins with "B"
		if i != 0: #if last byte is not completely filled, pad with 0's
			byte += "0"*(8-i) #padding
			i = 0
			line += byte + ", " #add to line and reset like normal
			byte = "B"
		if lineindex == srcimage.height-1: line = line [:-2] #remove last comma if the last line
		stdout.write(line+"\n") #output our handiwork
	stdout.write("},\n\n")
	
input("\nDone.")
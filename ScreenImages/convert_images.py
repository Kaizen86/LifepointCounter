from PIL import Image
from os import listdir

def mean(values: list[int]):
  """Returns the mean average from a list of numbers"""
  # The mean average is the sum of some numbers, divided by their quantity.
  return sum(values) / len(values)

# Get the list of image files in this directory
files = [i for i in listdir() if i.endswith(".png")]

for filename in files:
  print(filename)
  # Load the source image as RGB
  srcimage = Image.open(filename).convert('RGB')

  imagebits = []
  for Y in range(0,srcimage.height):
    line = []
    for X in range(0,srcimage.width):
      #Get RGB and compute mean brightness of the RGB components
      R,G,B = srcimage.getpixel((X,Y))
      brightness = mean([R,G,B])
      line.append(brightness >= 128)
    imagebits.append(line)

  # This section converts the booleans into a C source code array of bytes
  i = 0
  byte = "B"
  print("{")
  for lineindex, linebits in enumerate(imagebits):
    line = "\t" # Indent line
    for bit in linebits:
      byte += "1" if bit else "0" # Append bit
      i += 1 #keep track of how many bits that byte has
      if i == 8: # Do we have 8 bits? If so, add that byte to the line and start a new one
        line += byte + ", " # Append byte to line
        i = 0 # Reset bit counter
        byte = "B" # Each byte begins with "B"

    if i != 0: # If the last byte is not completely filled, pad with 0's
      byte += "0"*(8-i) # Generate and append padding
      # Prepare for the next byte just as before
      i = 0
      line += byte + ", "
      byte = "B"

    # If we're on the last line, remove the trailing comma.
    if lineindex == srcimage.height-1:
      line = line [:-2]

    # Finally, print the finished line.
    print(line)

  # At the end of an image, print the closing array brace, ready for the next file.
  print("},\n")

input("\nDone.")

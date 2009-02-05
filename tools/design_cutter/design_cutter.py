#!/usr/bin/python

#Usage
#
# This program cuts an image in 9 part
# syntax :
# design_cutter.py size image
# example :
# design_cutter.py 20 big_hide.png

import os, sys
import Image

def sub_box(image, left, upper, right, lower):
    box = (left, upper, right, lower)
    return image.crop(box)

i = 0
for arg in sys.argv:
    if i == 1:
        size = int(arg)
    if i == 2:
        file = arg
    i+=1

if i!=3:
    print "2 arguments needed"
    exit()

prefix = os.path.splitext(file)[0]

im = Image.open(file)
xsize, ysize = im.size

try:

    #Corner
    out = sub_box(im, 0, 0, size, size)
    out.save(prefix+"_top-left.png", "PNG")
    print prefix+"_top-left.png generated"
    out = sub_box(im, xsize-size, 0, xsize, size)
    out.save(prefix+"_top-right.png", "PNG")
    print prefix+"_top-left.png generated"
    out = sub_box(im, 0, ysize-size, size, ysize)
    out.save(prefix+"_bottom-left.png", "PNG")
    print prefix+"_top-left.png generated"
    out = sub_box(im, xsize-size, ysize-size, xsize,  ysize)
    out.save(prefix+"_bottom-right.png", "PNG")
    print prefix+"_top-left.png generated"

    #Border
    out = sub_box(im, size, 0, xsize-size,  size)
    out.save(prefix+"_top.png", "PNG")
    print prefix+"_top.png generated"
    out = sub_box(im, size,ysize-size , xsize-size, ysize)
    out.save(prefix+"_bottom.png", "PNG")
    print prefix+"_bottom.png generated"
    out = sub_box(im, 0, size, size, ysize-size)
    out.save(prefix+"_left.png", "PNG")
    print prefix+"_left.png generated"
    out = sub_box(im, xsize-size, size, xsize,  ysize-size)
    out.save(prefix+"_rigth.png", "PNG")
    print prefix+"_right.png generated"

    #Center 
    out = sub_box(im, size, size, xsize-size,  ysize-size)
    out.save(prefix+"_center.png", "PNG")
    print prefix+"_center.png generated"

except IOError:
    print "cannot create all image", infile


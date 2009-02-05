#!/usr/bin/python

#Usage
#
# This program cuts an image in 9 part
# syntax :
# design_cutter.py width heigth image
# example :
# design_cutter.py 10 15 big_hide.png

import os, sys
import Image

def sub_box(image, left, upper, right, lower):
    box = (left, upper, right, lower)
    return image.crop(box)

i = 0
for arg in sys.argv:
    if i == 1:
        w = int(arg)
    if i == 2:
        h = int(arg)
    if i == 3:
        file = arg
    i+=1

if i!=4:
    print "3 arguments needed"
    exit()

prefix = os.path.splitext(file)[0]

im = Image.open(file)
xsize, ysize = im.size

try:

    #Corner
    out = sub_box(im, 0, 0, w, h)
    out.save(prefix+"_top-left.png", "PNG")
    print prefix+"_top-left.png generated"
    out = sub_box(im, w+1, 0, xsize, h)
    out.save(prefix+"_top-right.png", "PNG")
    print prefix+"_top-left.png generated"
    out = sub_box(im, 0, h+1, w, ysize)
    out.save(prefix+"_bottom-left.png", "PNG")
    print prefix+"_top-left.png generated"
    out = sub_box(im, w+1, h+1, xsize,  ysize)
    out.save(prefix+"_bottom-right.png", "PNG")
    print prefix+"_top-left.png generated"

    #Border
    out = sub_box(im, w, 0, w+1,  h)
    out.save(prefix+"_top.png", "PNG")
    print prefix+"_top.png generated"
    out = sub_box(im, w,h+1 , w+1, ysize)
    out.save(prefix+"_bottom.png", "PNG")
    print prefix+"_bottom.png generated"
    out = sub_box(im, 0, h, w, h+1)
    out.save(prefix+"_left.png", "PNG")
    print prefix+"_left.png generated"
    out = sub_box(im, w+1, h, xsize, h+1)
    out.save(prefix+"_rigth.png", "PNG")
    print prefix+"_right.png generated"

    #Center 
    out = sub_box(im,w, h, w+1, h+1)
    out.save(prefix+"_center.png", "PNG")
    print prefix+"_center.png generated"

except IOError:
    print "cannot create all image", infile


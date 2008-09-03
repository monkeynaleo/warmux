PATCH=	box2d_fixed.patch

all:
	(cd Source; make)


clean:
	(cd Source; make clean)

patch:
	svn diff > $(PATCH)
#	- diff --unified /dev/null Makefile >> $(PATCH)
#	- diff --unified /dev/null Documentation/latex/Makefile >> $(PATCH)
#	- diff --unified /dev/null Contrib/freeglut/Makefile >> $(PATCH)
#	- diff --unified /dev/null Contrib/glui/Makefile >> $(PATCH)
#	- diff --unified /dev/null Source/Makefile >> $(PATCH)
#	- diff --unified /dev/null Examples/TestBed/Makefile >> $(PATCH)
#	- diff --unified /dev/null Source/Common/Fixed.h >> $(PATCH)
#	- diff --unified /dev/null Source/Common/jtypes.h >> $(PATCH)
#	- diff --unified /dev/null Source/Contrib/b2Polygon.cpp >> $(PATCH)
#	- diff --unified /dev/null Source/Contrib/b2Polygon.h >> $(PATCH)
#	- diff --unified /dev/null Source/Contrib/b2Triangle.cpp >> $(PATCH)
#	- diff --unified /dev/null Source/Contrib/b2Triangle.h >> $(PATCH)

CFLAGS = -g -Wall -fPIC -std=c18

OBJS = \
  rect.o \
  circ.o

all:		libgeom.so  libgeom.a testgeom-static testgeom-dynamic

libgeom.so:	$(OBJS)
		$(CC) -shared $(OBJS) -o $@

libgeom.a:	$(OBJS)
		ar rcs $@ $(OBJS)

testgeom-static: testgeom.o
		$(CC)  -static testgeom.o -L. -lgeom -o $@

testgeom-dynamic: testgeom.o
		$(CC) testgeom.o -L. -lgeom -o $@

.PHONY:		clean
clean:
		rm -f *.o *.so *.a testgeom-*
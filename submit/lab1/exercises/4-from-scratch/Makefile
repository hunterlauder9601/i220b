PROJECT = key-value

SRC_FILES = README Makefile process-file.h process-file.c main.c key-value.h key-value.c
CFLAGS = -g -Wall -std=c18
LDFLAGS =

$(PROJECT):	main.o process-file.o key-value.o
		$(CC) main.o process-file.o key-value.o $(LDFLAGS) -o $@

clean:
		rm -f *.o $(PROJECT) $(PROJECT).tar.gz

depend:
		$(CC) -MM $(CPPFLAGS) *.c

dist: $(PROJECT).tar.gz

$(PROJECT).tar.gz: $(SRC_FILES)
		tar -cf $(PROJECT).tar $(SRC_FILES)
		gzip -f $(PROJECT).tar

#key-value.o: key-value.c key-value.h
#main.o: main.c process-file.h


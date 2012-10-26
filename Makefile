SOURCES= main.c mobi.c pdb.c exth.c utils.c mobi_file.c compress.c
HEADERS= pdb.h mobi.h exth.h utils.h bytestream.h mobi_file.h compress.h

OBJECTS=$(SOURCES:.c=.o)
CFLAGS= -Wall -g

all: readmobi

readmobi: $(OBJECTS) $(HEADERS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	rm -f readmobi $(OBJECTS)

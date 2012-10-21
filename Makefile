SOURCES= main.c mobi.c pdb.c exth.c
HEADERS= pdb.h mobi.h exth.h

OBJECTS=$(SOURCES:.c=.o)
CFLAGS= -Wall -g

all: readmobi

readmobi: $(OBJECTS) $(HEADERS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	rm -f readmobi $(OBJECTS)

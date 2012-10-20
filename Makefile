SOURCES= main.c mobi.c pdb.c
HEADERS= pdb.h mobi.h

OBJECTS=$(SOURCES:.c=.o)
CFLAGS= -Wall

all: readmobi

readmobi: $(OBJECTS) $(HEADERS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	rm -f readmobi $(OBJECTS)

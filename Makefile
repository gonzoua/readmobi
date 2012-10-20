SOURCES= main.c pdb.c
HEADERS= pdb.h

OBJECTS=$(SOURCES:.c=.o)
CFLAGS= -Wall

all: readmobi

readmobi: $(OBJECTS) $(HEADERS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	rm -f readmobi $(OBJECTS)

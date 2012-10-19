SOURCES= main.c
HEADERS= pdb.h

OBJECTS=$(SOURCES:.c=.o)

all: readmobi

readmobi: $(OBJECTS) $(HEADERS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	rm -f readmobi $(OBJECTS)

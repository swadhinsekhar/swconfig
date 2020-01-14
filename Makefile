CFLAGS = -g -Wall -fPIC
CFLAGS += -I./lib/rtl8367c
LDFLAGS += -lrtl8367c

OBJ= swconfig.o

main: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o swconfig $(LDFLAGS)
	rm -f *.o

clean:
	rm -f *.o swconfig


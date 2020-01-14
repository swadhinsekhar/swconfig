OBJ = $(patsubst %.c, %.o, $(wildcard *.c))

INCL_PATH =-I.                               \
           -DPTRACING=1 -D_REENTRANT -fno-exceptions -fPIC -DPIC
EXTRA_CFLAGS += -g -Wall -Wstrict-prototypes -Dlinux -DLINUX -DMDC_MDIO_OPERATION -DRTK_X86_CLE
EXTRA_CFLAGS += -I./

DEFAULT_LIBDIR=/usr/local/lib

SOTARGET=librtl8367c.so

%.o: %.c
	$(CC) $(CFLAGS) -fPIC $(INCL_PATH) $(EXTRA_CFLAGS) -c -o $@ $<

all: $(SOTARGET)

$(SOTARGET): $(OBJ)
	$(CC) $(CFLAGS) -shared -fPIC $(INCL_PATH) $(EXTRA_CFLAGS) -o $@ $^ $(LDFLAGS)
	rm -f *.o

install:
	cp $(SOTARGET) $(DEFAULT_LIBDIR)
	ldconfig

clean:
	rm -f *.so *.o *.a



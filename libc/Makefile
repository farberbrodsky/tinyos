CFLAGS?=-O2
CPPFLAGS?=
LDFLAGS?=
LIBS?=

ARCHDIR=arch/i386

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -D__is_libc -Iinclude
CPPFLAGS:=$(CPPFLAGS) -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -D__is_libc -Iinclude
LIBK_CFLAGS:=$(CFLAGS)
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__is_libk

INCLUDEDIR?=$(PREFIX)/include

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(ARCH_CPPFLAGS)
LIBK_CFLAGS:=$(LIBK_CFLAGS) $(KERNEL_ARCH_CFLAGS)
LIBK_CPPFLAGS:=$(LIBK_CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)

FREEOBJS=\
$(ARCH_FREEOBJS) \
stdio/puts.o \
stdlib/abort.o \
string/mem.o \
string/str.o \

HOSTEDOBJS=\
$(ARCH_HOSTEDOBJS) \

OBJS=\
$(FREEOBJS) \
$(HOSTEDOBJS) \

LIBK_OBJS=$(FREEOBJS:.o=.libk.o)

BINARIES=libc.a libk.a

.PHONY: all clean install-headers install
.SUFFIXES: .o .libk.o .c .S

all: $(BINARIES)
libc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CFLAGS)

.c.libk.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS) $(LIBK_CFLAGS)

.S.libk.o:
	$(CC) -MD -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CFLAGS)

clean:
	rm -f $(BINARIES) *.a
	rm -f $(OBJS) $(LIBK_OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) $(LIBK_OBJS:.o=.d) *.d */*.d */*/*.d

-include $(OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)

install-headers:
	mkdir -p $(DESTDIR)/include
	cp -R --preserve=timestamps include/. $(DESTDIR)/include/.

install: $(BINARIES)
	mkdir -p $(DESTDIR)/lib
	cp $(BINARIES) $(DESTDIR)/lib

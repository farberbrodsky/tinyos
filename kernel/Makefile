CFLAGS?=-O2
CFLAGS:=$(CFLAGS) -Wall -Wextra -ffreestanding -I$(DESTDIR)/include
CPPFLAGS?=-O2
CPPFLAGS:=$(CPPFLAGS) -Wall -Wextra -ffreestanding -fno-exceptions -fno-rtti -D__is_kernel -I$(DESTDIR)/include -L$(DESTDIR)/lib
LDFLAGS?=
LIBS?=
LIBS:=$(LIBS) -nostdlib -lk -lgcc

QEMU=qemu-system-i386
ARCHDIR=arch/i386

include $(ARCHDIR)/make.config
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
util/kassert.o \
util/str_util.o \
core/main.o

OBJS=\
$(ARCHDIR)/crti.o \
$(KERNEL_OBJS) \
$(ARCHDIR)/crtn.o \
 
LINK_LIST=\
$(LDFLAGS) \
$(ARCHDIR)/crti.o \
$(KERNEL_OBJS) \
$(LIBS) \
$(ARCHDIR)/crtn.o \

.PHONY: all clean qemu install-headers
.SUFFIXES: .o .cpp .S

all: tinyos.bin

tinyos.bin: $(OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CPPFLAGS) $(LINK_LIST)
	grub-file --is-x86-multiboot tinyos.bin

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CPPFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

.cpp.o:
	$(CC) -MD -c $< -o $@ -std=c++17 $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CPPFLAGS)

clean:
	rm -f tinyos.bin
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install-headers:
	mkdir -p $(DESTDIR)/include
	cp -R --preserve=timestamps include/. $(DESTDIR)/include/.

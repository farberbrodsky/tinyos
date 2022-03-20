/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(_start)

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
    /* the kernel will be in 3GB + 1MB, but it starts at 1MB because bootloaders */
    . = 0x00100000;

    _kernel_start = .;
    .multiboot.data : {
        *(.multiboot.data)
    }
    .multiboot.text : {
        *(.multiboot.text)
    }

    . += 0xC0000000;
    /* Add a symbol that indicates the start address of the kernel. */
    .text ALIGN (4K) : AT (ADDR (.text) - 0xC0000000)
    {
        *(.text)
    }

    .rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xC0000000)
    {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000)
    {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
    }

    /* Add a symbol that indicates the end address of the kernel. */
    _kernel_end = .;
}


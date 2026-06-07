ENTRY(kmain)

SECTIONS
{
    . = 0xffffffff80000000;

    .text : {
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .limine_requests_start_marker : {
        KEEP(*(.limine_requests_start))
    }

    .limine_requests : {
        KEEP(*(.limine_requests))
    }

    .limine_requests_end_marker : {
        KEEP(*(.limine_requests_end))
    }

    .data : {
        *(.data .data.*)
    }

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    }
}

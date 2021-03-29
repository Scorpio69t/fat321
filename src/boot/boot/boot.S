
#include <kernel/multiboot.h>

.section ".multiboot_header"

	.align  MULTIBOOT_HEADER_ALIGN
multiboot_header:
        .long   MULTIBOOT2_HEADER_MAGIC
        .long   GRUB_MULTIBOOT_ARCHITECTURE_I386
        .long   multiboot_header_end - multiboot_header
	/* 校验和 */
        .long   -(MULTIBOOT2_HEADER_MAGIC+GRUB_MULTIBOOT_ARCHITECTURE_I386+(multiboot_header_end - multiboot_header))

        .align MULTIBOOT_HEADER_ALIGN
        .short MULTIBOOT_HEADER_TAG_END
        .short 0
        .long 8
multiboot_header_end:

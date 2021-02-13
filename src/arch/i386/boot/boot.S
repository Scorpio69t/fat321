
#include <asm/multiboot.h>

.section ".multiboot_header"

	.align  MULTIBOOT_HEADER_ALIGN
multiboot_header:
        .long   MULTIBOOT2_HEADER_MAGIC
        .long   GRUB_MULTIBOOT_ARCHITECTURE_I386
        .long   multiboot_header_end - multiboot_header
	/* 校验和 */
        .long   -(MULTIBOOT2_HEADER_MAGIC+GRUB_MULTIBOOT_ARCHITECTURE_I386+(multiboot_header_end - multiboot_header))


       /*.align 	MULTIBOOT_HEADER_ALIGN
framebuffer_tag_start:
        .short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
        .short MULTIBOOT_HEADER_TAG_OPTIONAL
        .long framebuffer_tag_end - framebuffer_tag_start
        .long 1920
        .long 1080
        .long 32
framebuffer_tag_end:*/

        .align MULTIBOOT_HEADER_ALIGN
        .short MULTIBOOT_HEADER_TAG_END
        .short 0
        .long 8
multiboot_header_end:

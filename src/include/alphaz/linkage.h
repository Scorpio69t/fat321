#ifndef _ALPHAZ_LINKAGE_H_
#define _ALPHAZ_LINKAGE_H_

#ifndef __ALIGN
#define __ALIGN		.align 4,0x90
#define __ALIGN_STR	".align 4,0x90"
#endif

#ifndef ENTRY
#define ENTRY(name)         \
.global name;               \
__ALIGN;                    \
name:
#endif

#ifndef END
#define END(name) \
  .size name, .-name
#endif

#endif

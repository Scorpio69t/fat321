#ifndef _ALPHAZ_FONTDATA_H_
#define _ALPHAZ_FONTDATA_H_

struct font_desc {
    const char *name;
    int width, height;
    const unsigned char *font;
};

extern const struct font_desc font_8x16;

#endif

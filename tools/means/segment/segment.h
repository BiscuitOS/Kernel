#ifndef _SEGMENT_H
#define _SEGMENT_H

struct seg_desc
{
    unsigned long limit; /* optional alignment */
    unsigned long base;
    unsigned char type;
    unsigned char dpl;
    unsigned char flag;
};
#endif

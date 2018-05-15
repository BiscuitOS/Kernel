#ifndef _ASM_IO_H
#define _ASM_IO_H

/*
 * This file contains the definitions for the x86 IO instructions
 * inb/inw/inl/outb/outw/outl and the "string versions" of the same
 * (insb/insw/insl/outsb/outsw/outsl). You can also use "pausing"
 * versions of the single-IO instructions (inb_p/inw_p/..).
 *
 * This file is not meant to be obfuscating: it's just complicated
 * to (a) handle it all in a way that makes gcc able to optimize it
 * as well as possible and (b) trying to avoid writing the same thing
 * over and over again with slight variations and possibly making a
 * mistake somewhere.
 */

/*
 * Thanks to James van Artsdalen for a better timing-fix than
 * the two short jumps: using outb's to a nonexistent port seems
 * to guarantee better timings even on fast machines.
 *
 * On the other hand, I'd like to be sure of a non-existent port:
 * I feel a bit unsafe about using 0x80 (should be safe, though)
 *
 *		Linus
 */

#ifdef SLOW_IO_BY_JUMPING
#define __SLOW_DOWN_IO __asm__ __volatile__("jmp 1f\n1:\tjmp 1f\n1:")
#else
#define __SLOW_DOWN_IO __asm__ __volatile__("outb %al,$0x80")
#endif

#ifdef REALLY_SLOW_IO
#define SLOW_DOWN_IO { __SLOW_DOWN_IO; __SLOW_DOWN_IO; __SLOW_DOWN_IO; __SLOW_DOWN_IO; }
#else
#define SLOW_DOWN_IO __SLOW_DOWN_IO
#endif

/*
 * Talk about misusing macros..
 */

#define __OUT1(s,x) \
static inline void __out##s(unsigned x value, unsigned short port) {

#define __OUT2(s,s1,s2) \
__asm__ __volatile__ ("out" #s " %" s1 "0,%" s2 "1"

#define __OUT(s,s1,x) \
__OUT1(s,x) __OUT2(s,s1,"w") : : "a" (value), "d" (port)); } \
__OUT1(s##c,x) __OUT2(s,s1,"") : : "a" (value), "i" (port)); } \
__OUT1(s##_p,x) __OUT2(s,s1,"w") : : "a" (value), "d" (port)); SLOW_DOWN_IO; } \
__OUT1(s##c_p,x) __OUT2(s,s1,"") : : "a" (value), "i" (port)); SLOW_DOWN_IO; }

#define __IN1(s) \
static inline unsigned int __in##s(unsigned short port) { unsigned int _v;

#define __IN2(s,s1,s2) \
__asm__ __volatile__ ("in" #s " %" s2 "1,%" s1 "0"

#define __IN(s,s1,i...) \
__IN1(s) __IN2(s,s1,"w") : "=a" (_v) : "d" (port) ,##i ); return _v; } \
__IN1(s##c) __IN2(s,s1,"") : "=a" (_v) : "i" (port) ,##i ); return _v; } \
__IN1(s##_p) __IN2(s,s1,"w") : "=a" (_v) : "d" (port) ,##i ); SLOW_DOWN_IO; return _v; } \
__IN1(s##c_p) __IN2(s,s1,"") : "=a" (_v) : "i" (port) ,##i ); SLOW_DOWN_IO; return _v; }

#define __INS(s) \
static inline void ins##s(unsigned short port, void * addr, unsigned long count) \
{ __asm__ __volatile__ ("cld ; rep ; ins" #s \
: "=D" (addr), "=c" (count) : "d" (port),"0" (addr),"1" (count)); }

#define __OUTS(s) \
static inline void outs##s(unsigned short port, const void * addr, unsigned long count) \
{ __asm__ __volatile__ ("cld ; rep ; outs" #s \
: "=S" (addr), "=c" (count) : "d" (port),"0" (addr),"1" (count)); }

//__IN(b,"b","0" (0))
//__IN(w,"w","0" (0))
//__IN(l,"")

//__OUT(b,"b",char)
//__OUT(w,"w",short)
//__OUT(l,,int)

__INS(b)
__INS(w)
__INS(l)

__OUTS(b)
__OUTS(w)
__OUTS(l)

/* On BiscuitOs need extend function macro */
//__OUT(b,"b",char)

static inline void __outb(unsigned char value, unsigned short port) 
{ 
    __asm__ __volatile__ ("outb %b0,%w1" : : "a" (value), "d" (port)); 
}

static inline void __outbc(unsigned char value, unsigned short port) 
{
    __asm__("outb %%al, %%dx" : : "a" (value), "d" (port)); 
}

static inline void __outb_p(unsigned char value, unsigned short port) 
{
    __asm__ __volatile__ ("out" "b" " %" "b" "0,%" "w" "1" : : "a" (value), "d" (port));
    SLOW_DOWN_IO; 
}
static inline void __outbc_p(unsigned char value, unsigned short port) 
{
    __asm__("outb %%al, %%dx" : : "a" (value), "d" (port));
    SLOW_DOWN_IO; 
}

// __OUT(w, "w", short)

static inline void __outw(unsigned short value, unsigned short port)
{
    __asm__ __volatile__ ("out" "w" " %" "w" "0,%" "w" "1" : : "a" (value), "d" (port));
}

static inline void __outwc(unsigned short value, unsigned short port)
{
    __asm__ __volatile__ ("out" "w" " %" "w" "0,%" "" "1" : : "a" (value), "i" (port));
}

static inline void __outw_p(unsigned short value, unsigned short port)
{
    __asm__ __volatile__ ("out" "w" " %" "w" "0,%" "w" "1" : : "a" (value), "d" (port));
    SLOW_DOWN_IO; 
}
static inline void __outwc_p(unsigned short value, unsigned short port) 
{
    __asm__ __volatile__ ("out" "w" " %" "w" "0,%" "" "1" : : "a" (value), "i" (port));
    SLOW_DOWN_IO; 
}

// __OUT(l,,int)

static inline void __outl(unsigned long value, unsigned short port)
{
    __asm__ __volatile__ ("out" "l" " %" "" "0,%" "w" "1" : : "a" (value), "d" (port));
}

static inline void __outlc(unsigned long value, unsigned short port)
{
    __asm__ __volatile__ ("out" "l" " %" "" "0,%" "" "1" : : "a" (value), "i" (port));
}

static inline void __outl_p(unsigned long value, unsigned short port)
{
    __asm__ __volatile__ ("out" "l" " %" "" "0,%" "w" "1" : : "a" (value), "d" (port));
    SLOW_DOWN_IO;
}
static inline void __outlc_p(unsigned long value, unsigned short port)
{
    __asm__ __volatile__ ("out" "l" " %" "" "0,%" "" "1" : : "a" (value), "i" (port));
    SLOW_DOWN_IO;
}

//__IN(b,"b","0" (0))

static inline unsigned int __inb(unsigned short port) 
{ 
    unsigned int _v; 

    __asm__ __volatile__ ("in" "b" " %" "w" "1,%" "b" "0" : "=a" (_v) : "d" (port) , "0" (0) );
    return _v; 
}

static inline unsigned int __inbc(unsigned short port)
{
    unsigned int _v; 

    __asm__ __volatile__ ("inb %%dx, %%al": "=a" (_v) : "d" (port) , "0" (0));
    return _v; 
}

static inline unsigned int __inb_p(unsigned short port)
{
    unsigned int _v;

    __asm__ __volatile__ ("in" "b" " %" "w" "1,%" "b" "0" : "=a" (_v) : "d" (port) ,"0" (0)); 
    SLOW_DOWN_IO; 
    return _v;
}

static inline unsigned int __inbc_p(unsigned short port) 
{
    unsigned int _v;

    __asm__ __volatile__ ("inb %%dx, %%al": "=a" (_v) : "d" (port) , "0" (0));
    SLOW_DOWN_IO;
    return _v;
}

//__IN(w,"w","0" (0))

static inline unsigned int __inw(unsigned short port)
{
    unsigned int _v;

    __asm__ __volatile__ ("in" "w" " %" "w" "1,%" "w" "0" : "=a" (_v) : "d" (port) , "0" (0) );
    return _v;
}

static inline unsigned int __inwc(unsigned short port)
{
    unsigned int _v;

    __asm__ __volatile__ ("in" "w" " %" "" "1,%" "w" "0" : "=a" (_v) : "i" (port) , "0" (0));
    return _v;
}

static inline unsigned int __inw_p(unsigned short port)
{
    unsigned int _v;

    __asm__ __volatile__ ("in" "w" " %" "w" "1,%" "w" "0" : "=a" (_v) : "d" (port) ,"0" (0));
    SLOW_DOWN_IO;
    return _v;
}

static inline unsigned int __inwc_p(unsigned short port)
{
    unsigned int _v;

    __asm__ __volatile__ ("in" "w" " %" "" "1,%" "w" "0" : "=a" (_v) : "i" (port) , "0" (0));
    SLOW_DOWN_IO;
    return _v;
}


//__IN(l,"")

static inline unsigned int __inl(unsigned short port)
{
    unsigned int _v;

    __asm__ __volatile__ ("in" "l" " %" "w" "1,%" "" "0" : "=a" (_v) : "d" (port));
    return _v;
}

static inline unsigned int __inlc(unsigned short port)
{
    unsigned int _v;

    __asm__ __volatile__ ("in" "l" " %" "" "1,%" "" "0" : "=a" (_v) : "i" (port));
    return _v;
}

static inline unsigned int __inl_p(unsigned short port)
{
    unsigned int _v;

    __asm__ __volatile__ ("in" "l" " %" "w" "1,%" "" "0" : "=a" (_v) : "d" (port));
    SLOW_DOWN_IO;
    return _v;
}

static inline unsigned int __inlc_p(unsigned short port)
{
    unsigned int _v;

    __asm__ __volatile__ ("in" "l" " %" "" "1,%" "" "0" : "=a" (_v) : "i" (port));
    SLOW_DOWN_IO;
    return _v;
}

/*
 * Note that due to the way __builtin_constant_p() works, you
 *  - can't use it inside a inline function (it will never be true)
 *  - you don't have to worry about side effects within the __builtin..
 */
#define outb(val,port) \
((__builtin_constant_p((port)) && (port) < 256) ? \
	__outbc((val),(port)) : \
	__outb((val),(port)))

#define inb(port) \
((__builtin_constant_p((port)) && (port) < 256) ? \
	__inbc(port) : \
	__inb(port))

#define outb_p(val,port) \
((__builtin_constant_p((port)) && (port) < 256) ? \
	__outbc_p((val),(port)) : \
	__outb_p((val),(port)))

#define inb_p(port) \
((__builtin_constant_p((port)) && (port) < 256) ? \
	__inbc_p(port) : \
	__inb_p(port))

#define outw(val,port) \
((__builtin_constant_p((port)) && (port) < 256) ? \
	__outwc((val),(port)) : \
	__outw((val),(port)))

#define inw(port) \
((__builtin_constant_p((port)) && (port) < 256) ? \
	__inwc(port) : \
	__inw(port))

#define outw_p(val,port) \
((__builtin_constant_p((port)) && (port) < 256) ? \
	__outwc_p((val),(port)) : \
	__outw_p((val),(port)))

#define inw_p(port) \
((__builtin_constant_p((port)) && (port) < 256) ? \
	__inwc_p(port) : \
	__inw_p(port))

#define outl(val,port) \
((__builtin_constant_p((port)) && (port) < 256) ? \
	__outlc((val),(port)) : \
	__outl((val),(port)))

#define inl(port) \
((__builtin_constant_p((port)) && (port) < 256) ? \
	__inlc(port) : \
	__inl(port))

#define outl_p(val,port) \
((__builtin_constant_p((port)) && (port) < 256) ? \
	__outlc_p((val),(port)) : \
	__outl_p((val),(port)))

#define inl_p(port) \
((__builtin_constant_p((port)) && (port) < 256) ? \
	__inlc_p(port) : \
	__inl_p(port))

#endif

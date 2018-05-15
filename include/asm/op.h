
//__OUT(b,"b",char)

static inline void __outb(unsigned char value, unsigned short port) 
{ 
    __asm__ __volatile__ ("out" "b" " %" "b" "0,%" "w" "1" : : "a" (value), "d" (port)); 
}

static inline void __outbc(unsigned char value, unsigned short port) 
{
    __asm__ __volatile__ ("out" "b" " %" "b" "0,%" "" "1" : : "a" (value), "i" (port)); 
}

static inline void __outb_p(unsigned char value, unsigned short port) 
{
    __asm__ __volatile__ ("out" "b" " %" "b" "0,%" "w" "1" : : "a" (value), "d" (port));
    SLOW_DOWN_IO; 
}
static inline void __outbc_p(unsigned char value, unsigned short port) 
{
    __asm__ __volatile__ ("out" "b" " %" "b" "0,%" "" "1" : : "a" (value), "i" (port));
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

    __asm__ __volatile__ ("in" "b" " %" "" "1,%" "b" "0" : "=a" (_v) : "i" (port) , "0" (0));
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

    __asm__ __volatile__ ("in" "b" " %" "" "1,%" "b" "0" : "=a" (_v) : "i" (port) , "0" (0));
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


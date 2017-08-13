/*
 * hd.c
 *
 * (C) 1991 Linus Torvalds
 */
#include <asm/io.h>
#include <asm/system.h>
#include <linux/head.h>
#include <linux/fs.h>
#include <linux/config.h>
#include <linux/hdreg.h>
#include <linux/kernel.h>
#include <linux/sched.h>

#include <asm/system.h>

#include <linux/blk.h>

/* Max read/write errors/sector */
#define MAX_ERRORS    7
#define MAX_HD        2

#define MAJOR_NR            3
#define DEVICE_NAME         "harddisk"
#define DEVICE_NR(device)   (MINOR(device) / 5)

#define CURRENT             (blk_dev[MAJOR_NR].current_request)
#define CURRENT_DEV         DEVICE_NR(CURRENT->dev)

#define INIT_REQUEST \
repeat:              \
    if (!CURRENT)    \
        return;      \
    if (MAJOR(CURRENT->dev) != MAJOR_NR)  \
        panic(": request list destroyed"); \
    if (CURRENT->bh) {     \
        panic(": block not locked");       \
    }

#define CMOS_READ(addr) ({\
outb_p(0x80|addr,0x70);\
inb_p(0x71);\
})

extern void rd_load(void);
static int recalibrate = 0;
static int reset = 0;
/*
 * This struct define the HD's and their types.
 */
struct hd_i_struct {
	int head, sect, cyl, wpcom, lzone, ctl;
};

#ifdef HD_TYPE
struct hd_i_struct hd_info[] = { HD_TYPE };

#define NR_HD ((sizeof(hd_info)) / (sizeof(struct hd_i_struct)))
#else
struct hd_i_struct hd_info[] = { {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0} };

static int NR_HD = 0;
#endif

static struct hd_struct {
	long start_sect;
	long nr_sects;
} hd[5 * MAX_HD] = { {
0, 0},};

#define port_read(port, buf, nr) \
	__asm__("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr))

#define port_write(port, buf, nr) \
	__asm__("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr))

extern void hd_interrupt(void);
static void (do_hd_request) (void);
void (*do_hd) (void) = NULL;

static inline void unlock_buffer(struct buffer_head *bh)
{
	if (!bh->b_lock)
		printk(": free buffer being unlocked\n");
	bh->b_lock = 0;
	wake_up(&bh->b_wait);
}

static inline void end_request(int uptodate)
{
	if (CURRENT->bh) {
		CURRENT->bh->b_uptodate = uptodate;
		unlock_buffer(CURRENT->bh);
	}
	if (!uptodate) {
		printk(" I/O error\n\r");
		printk("dev %04x, block %d\n\r", CURRENT->dev,
		       CURRENT->bh->b_blocknr);
	}
	wake_up(&CURRENT->waiting);
	wake_up(&wait_for_request);
	CURRENT->dev = -1;
	CURRENT = CURRENT->next;
}

static int drive_busy(void)
{
	unsigned int i;

	for (i = 0; i < 10000; i++)
		if (READY_STAT == (inb_p(HD_STATUS) & (BUSY_STAT | READY_STAT)))
			break;
	i = inb(HD_STATUS);
	i &= BUSY_STAT | READY_STAT | SEEK_STAT;
	if (i == (READY_STAT | SEEK_STAT))
		return (0);
	printk("HD controller times out\n\r");
	return (1);
}

static int controller_ready(void)
{
	int retries = 100000;

	while (--retries && (inb_p(HD_STATUS) & BUSY_STAT));
	return (retries);
}

static void hd_out(unsigned int drive, unsigned int nsect, unsigned int sect,
		   unsigned int head, unsigned int cyl, unsigned int cmd,
		   void (*intr_addr) (void))
{
	if (drive > 1 || head > 15)
		panic("trying to write bad sector");
	if (!controller_ready())
		panic("HD controller not ready");
	do_hd = intr_addr;
	outb_p(hd_info[drive].ctl, HD_CMD);
	outb_p(hd_info[drive].wpcom >> 2, HD_PRECOMP);
	outb_p(nsect, HD_NSECTOR);
	outb_p(sect, HD_SECTOR);
	outb_p(cyl, HD_LCYL);
	outb_p(cyl >> 8, HD_HCYL);
	outb_p(0xA0 | (drive << 4) | head, HD_CURRENT);
	outb(cmd, HD_COMMAND);
}

static void reset_controller(void)
{
	int i;

	outb(4, HD_CMD);
	for (i = 0; i < 100; i++)
		nop();

	outb(hd_info[0].ctl & 0x0f, HD_CMD);
	if (drive_busy())
		printk("HD-controller still busy\n\r");
	if ((inb(HD_ERROR)) != HD_NO_ERR)
		printk("HD-controller reset failed: %02x\n\r", i);
}

static int win_result(void)
{
	int i = inb_p(HD_STATUS);

	if ((i & (BUSY_STAT | READY_STAT | WRERR_STAT | SEEK_STAT | ERR_STAT))
	    == (READY_STAT | SEEK_STAT))
		return (0);	/* ok */
	if (i & 1)
		i = inb(HD_ERROR);
	return 1;
}

static void bad_rw_intr(void)
{
	if (++CURRENT->errors >= MAX_ERRORS)
		end_request(0);
	if (CURRENT->errors > MAX_ERRORS / 2)
		reset = 1;
}

static void recal_intr(void)
{
	if (win_result())
		bad_rw_intr();
	do_hd_request();
}

static void reset_hd(int nr)
{
	reset_controller();
	hd_out(nr, hd_info[nr].sect, hd_info[nr].sect, hd_info[nr].head - 1,
	       hd_info[nr].cyl, WIN_SPECIFY, &recal_intr);
}

static void write_intr(void)
{
	if (win_result()) {
		bad_rw_intr();
		do_hd_request();
		return;
	}
	if (--CURRENT->nr_sectors) {
		CURRENT->sector++;
		CURRENT->buffer += 512;
		do_hd = &write_intr;
		port_write(HD_DATA, CURRENT->buffer, 256);
		return;
	}
	end_request(1);
	do_hd_request();
}

static void read_intr(void)
{
	if (win_result()) {
		bad_rw_intr();
		do_hd_request();
		return;
	}
	port_read(HD_DATA, CURRENT->buffer, 256);
	CURRENT->errors = 0;
	CURRENT->buffer += 512;
	CURRENT->sector++;

	if (--CURRENT->nr_sectors) {
		do_hd = &read_intr;
		return;
	}
	end_request(1);
	do_hd_request();
}

void do_hd_request(void)
{
	int i, r = 0;
	unsigned int block, dev;
	unsigned int sec, head, cyl;
	unsigned int nsect;

repeat:
	if (!CURRENT)
		return;
	if (MAJOR(CURRENT->dev) != MAJOR_NR)
		panic(DEVICE_NAME ": request list destroyed");
	if (CURRENT->bh) {
		if (!CURRENT->bh->b_lock)
			panic(DEVICE_NAME ": block not locked");
	}

	dev = MINOR(CURRENT->dev);
	block = CURRENT->sector;

	if (dev >= 5 * NR_HD || block + 2 > hd[dev].nr_sects) {
		end_request(0);
		goto repeat;
	}
	block += hd[dev].start_sect;
	dev /= 5;
	__asm__("divl %4": "=a"(block), "=d"(sec):"0"(block), "1"(0),
		"r"(hd_info[dev].sect));
	__asm__("divl %4" : "=a"(cyl), "=d"(head) : "0"(block), "1"(0),
		"r"(hd_info[dev].head));
	sec++;
	nsect = CURRENT->nr_sectors;

	if (reset) {
		reset = 0;
		recalibrate = 1;
		reset_hd(CURRENT_DEV);
		return;
	}

	if (recalibrate) {
		recalibrate = 0;
		hd_out(dev, hd_info[CURRENT_DEV].sect, 0, 0, 0,
		       WIN_RESTORE, &recal_intr);
		return;
	}

	if (CURRENT->cmd == WRITE) {
		hd_out(dev, nsect, sec, head, cyl, WIN_WRITE, &write_intr);
		for (i = 0; i < 3000 && !(r = inb_p(HD_STATUS) & DRQ_STAT); i++)
			/* nothing */ ;
		if (!r) {
			bad_rw_intr();
			goto repeat;
		}
		port_write(HD_DATA, CURRENT->buffer, 256);
	} else if (CURRENT->cmd == READ) {
		hd_out(dev, nsect, sec, head, cyl, WIN_READ, &read_intr);
	} else
		panic("unknow hd-command");
}

void hd_init(void)
{
    blk_dev[MAJOR_NR].request_fn = do_hd_request;
    set_intr_gate(0x2E, &hd_interrupt);
    outb_p(inb_p(0x21) & 0xfb, 0x21);
    outb(inb_p(0xA1) & 0xbf, 0xA1);
}

/* This may be used only once, enforced by 'static int callable' */
int sys_setup(void *BIOS)
{
    static int callable = 1;
    int i, drive;
    unsigned char cmos_disks;
    struct partition *p;
    struct buffer_head *bh;

    if (!callable)
        return -1;
    callable = 0;
#ifndef HD_TYPE
    for (drive = 0; drive < 2; drive++) {
        hd_info[drive].cyl    = *(unsigned short *)BIOS;
        hd_info[drive].head   = *(unsigned char *)(2 + BIOS);
        hd_info[drive].wpcom  = *(unsigned short *)(5 + BIOS);
        hd_info[drive].ctl    = *(unsigned char *)(8 + BIOS);
        hd_info[drive].lzone  = *(unsigned short *)(12 + BIOS);
        hd_info[drive].sect   = *(unsigned char *)(14 + BIOS);
        BIOS += 16;
    }
    if (hd_info[1].cyl)
        NR_HD = 2;
    else
        NR_HD = 1;
#endif
    for (i = 0; i < NR_HD; i++) {
        hd[i * 5].start_sect = 0;
        hd[i * 5].nr_sects   = hd_info[i].head *
                               hd_info[i].sect * hd_info[i].cyl;
    }
    /*
     * We querry CMOS about hard disks: it could be that
     * we have a SCSI/ESDI/etc controller that is BIOS
     * compatable with ST-506, and thus showing up in our
     * BIOS table, but not register compatable, and therefore
     * not present in CMOS.
     *
     * Furthurmore, we will assume that our ST_506 drives
     * <if any> are the primary dirves in the system, and
     * the ones reflected as drive 1 or 2.
     *
     * The first drive is stored in the high nibble of CMOS
     * byte 0x12, the second in the low nibble. This will be
     * either a 4 bit drive type or 0xf indicating use byte 0x19
     * for an 8 bit type, dirve 1, 0x1a drive 2 in CMOS.
     *
     * Needless to say, a non-zero value means we have
     * an AT controller hard disk for that drive.
     */
    if ((cmos_disks = CMOS_READ(0x12)) & 0xf0)
        if (cmos_disks & 0x0f)
            NR_HD = 2;
        else
            NR_HD = 1;
    else
        NR_HD = 0;
    for (i = NR_HD; i < 2; i++) {
        hd[i * 5].start_sect = 0;
        hd[i * 5].nr_sects = 0;
    }
    for (drive = 0; drive < NR_HD; drive++) {
        if (!(bh = bread(0x300 + drive * 5, 0))) {
            printk("Unable to read partition table of drive %d\n\r",
                   drive);
            panic("");
        }
        if (bh->b_data[510] != 0x55 || (unsigned char)
            bh->b_data[510] != 0xAA) {
            printk("Bad partition table on drive %d\n\r", drive);
            panic("");
        }
        p = 0x1BE + (void *)bh->b_data;
        for (i = 1; i < 5; i++, p++) {
            hd[i + 5 * drive].start_sect = p->start_sect;
            hd[i + 5 * drive].nr_sects   = p->nr_sects;
        }
        brelse(bh);
    }
    if (NR_HD)
        printk("Partition table%s ok.\n\r", (NR_HD > 1) ? "s" : "");
    rd_load();
    mount_root();
    return 0;
}

void unexpected_hd_interrupt(void)
{
    printk("Unexpected HD interrupt\n\r");
}

#include "type.h"
#include "io.h"
#include "uart.h"
#include "print.h"
#include "str.h"
#include "nand.h"

#ifdef DEBUG_LEDS
#include "debug.h"
#endif /* DEBUG_LEDS */

/* nand flash standard command */
#define NAND_CMD_READ0      0   
#define NAND_CMD_READ1      1   
#define NAND_CMD_READ2      0x50 /* oob read */
#define NAND_CMD_READID     0x90
#define NAND_CMD_RESET      0xff
#define NAND_CMD_SEQIN      0x80
#define NAND_CMD_PAGEPROG0  0x10    /* true */
#define NAND_CMD_PAGEPROG1  0x11    /* dummy */
#define NAND_CMD_ERASE1     0x60
#define NAND_CMD_ERASE2     0xd0
#define NAND_CMD_STATUS     0x70
#define NAND_CMD_STATUS_MULTI   0x71

/* command control */
#define NF_CMD_CLE (1<<0)
#define NF_CMD_ALE (1<<1)

/* status bits */
#define NF_STATUS_READY (1<<6)
#define NF_STATUS_PASS (1<<0)

static struct mtd_info mtdinfo;

static void nf_wait_ready(struct mtd_info *mtd)
{
	while(!(readl(&mtd->nfctrl->nfstat) & 0x1))
		;
}

static inline void cmd_ctrl(struct mtd_info *mtd, int cmd, int ctrl)
{
	if(ctrl & NF_CMD_CLE)
		writeb(&mtd->nfctrl->nfcmmd, cmd & 0xFF);
	else
		writeb(&mtd->nfctrl->nfaddr, cmd & 0xFF);
}

static void nf_command(struct mtd_info *mtd, int cmd, int column, int page)
{
	int ctrl = NF_CMD_CLE;

	/* check and set pointer before program */
	if(cmd == NAND_CMD_SEQIN) {
		int readcmd;
		if(column >= mtd->writesize) {
			readcmd = NAND_CMD_READ2;
			column -= mtd->writesize;
		} else if(column < 256) {
			readcmd = NAND_CMD_READ0;
		} else {
			readcmd = NAND_CMD_READ1;
			column -= 256;
		}
		cmd_ctrl(mtd, readcmd, ctrl);
	}
	cmd_ctrl(mtd, cmd, ctrl);

	ctrl = NF_CMD_ALE;
	if(column != -1)
		cmd_ctrl(mtd, column, ctrl);

	if(page != -1) {
		cmd_ctrl(mtd, page      , ctrl);		/* A9 ~A16 */
		cmd_ctrl(mtd, page >>  8, ctrl);		/* A17~A24 */
		cmd_ctrl(mtd, page >> 16, ctrl);		/* A25~ */
	}

	ctrl = NF_CMD_CLE;
	switch(cmd) {
		case NAND_CMD_ERASE1:
			/* followed by erase2 */
			cmd_ctrl(mtd, NAND_CMD_ERASE2, ctrl);
		case NAND_CMD_PAGEPROG0:
		case NAND_CMD_STATUS:
			/* After NAND_CMD_ERASE1 or NAND_CMD_PAGEPROG0,
			 * we'll always check status.
			 */
			cmd_ctrl(mtd, NAND_CMD_STATUS, ctrl);
			return;
		case NAND_CMD_PAGEPROG1:
		case NAND_CMD_STATUS_MULTI:
		case NAND_CMD_SEQIN:
		case NAND_CMD_READID:
			return;

		case NAND_CMD_RESET:
		default:
			nf_wait_ready(mtd);
	}
}

/*
#define nf_write_byte(mtd, ch) writeb(&mtd->nfctrl->nfdata, (u8)(ch))
*/
static inline void nf_write_byte(struct mtd_info *mtd, u8 ch)
{
	writeb(&mtd->nfctrl->nfdata, ch);
}

static void nf_fill_oob(struct mtd_info *mtd, void *oob)
{
	/* oob size */
	u32 bytes = mtd->writesize >> 5;

	if(oob) {
		outsb((u32)&mtd->nfctrl->nfdata, oob, bytes);
	} else {
		while(bytes--)
			nf_write_byte(mtd, 0xFF);
	}
}

static void nf_write_page(struct mtd_info *mtd, void *data, void *oob)
{
	outsb((u32)&mtd->nfctrl->nfdata, data, mtd->writesize);
	nf_fill_oob(mtd, oob);
}

/*
 * 0x80 -> address & 528byte data -> 0x10 -> 0x70 -> read status
 */
static int nf_write(struct mtd_info *mtd, u32 offset, void *buf, u32 *len)
{
	u8 stat;
	u32 column, page, bytes;
	u8 *data = (u8 *)buf;

	if((offset + *len) > (1 << mtd->device_shift)) {
		puts("nf_write: invalid offset or length!\n");
		return -1;
	}

	column = offset & ((1 << mtd->page_shift) - 1);
	page = offset >> mtd->page_shift;


	while(*len) {
		bytes = mtd->writesize;
		nf_command(mtd, NAND_CMD_SEQIN, 0x0, page);

		/* clear leading bytes if offset not align to page */
		for(; column; column--)
			nf_write_byte(mtd, 0xFF);

		if(*len < mtd->writesize) {
			u32 i;
			bytes = *len;
			u32 remnant = mtd->writesize + (mtd->writesize >> 5) - bytes;

			for(i = bytes; i > 0; i--)
				nf_write_byte(mtd, *data++);
			/* clear both left bytes and oob */
			while(remnant--)
				nf_write_byte(mtd, 0xFF);
		} else {
			nf_write_page(mtd, data, NULL);
		}
		*len -= bytes;

		nf_command(mtd, NAND_CMD_PAGEPROG0, -1, -1);
		while(!((stat = readb(&mtd->nfctrl->nfdata)) & NF_STATUS_READY))
			;
		if(stat & NF_STATUS_PASS)
			puts("nf_write: write not passed!\n");
	}

	return 0;
}

/*
 * 0x60 -> block address -> 0xd0 -> read status -> i/o6=1? or RnB=1?
 * 	-> i/o0=0? -> error -> erase complete
 */
static int nf_erase(struct mtd_info *mtd, u32 offset, u32 len)
{
	u8 stat;
	u32 block;
	
	if((offset + len) > (1 << mtd->device_shift)) {
		puts("nf_erase: invalid offset or length!\n");
		return -1;
	}

	block = (offset >> mtd->page_shift) & ~((1 << (mtd->block_shift - mtd->page_shift)) - 1);

	while(len) {
		nf_command(mtd, NAND_CMD_ERASE1, -1, block);

		while(!((stat = readb(&mtd->nfctrl->nfdata)) & NF_STATUS_READY))
			;
		if(stat & NF_STATUS_PASS)
			puts("Error: erase not passed!\n");

		block++;
		if(len < ((1 << mtd->block_shift) - 1))
			len = 0;
		else
			len -= ((1 << mtd->block_shift) - 1);
	}

	return 0;
}

static int nf_read(struct mtd_info *mtd, u32 offset, void *data, u32 *len)
{
	/*TODO:*/
	return 0;
}

static void hw_setup(struct mtd_info *mtd)
{
	mtd->nfctrl = (struct nf_ctrl *)0x4E000000;
	/* tacls, twrph0, twrph1 */
	writel(&mtd->nfctrl->nfconf, 1<<12 | 4<<8 | 1<<4);
	writel(&mtd->nfctrl->nfcont, 0<<1 | 1<<0);
	/*TODO*/
}

inline struct mtd_info *get_mtd_info(void)
{
	return &mtdinfo;
}

void nand_setup(void)
{
	struct mtd_info *mtd = get_mtd_info();

	hw_setup(mtd);

//	mtd->oobsize = mtd->writesize >> 5;
	mtd->writesize = 512;
//	mtd->blocksize = 512*32;
//	mtd->planesize = 512*32*1024;

	/* based on address */
	mtd->page_shift = 9;
	mtd->block_shift = 14;
	mtd->device_shift = 25;

	mtd->nf_read = 0;
	mtd->nf_write = nf_write;
	mtd->nf_erase = nf_erase;
}

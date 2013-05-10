#include "type.h"
#include "io.h"
#include "uart.h"

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

/*0x4E000000*/
struct nf_ctrl {
	u32 nfconf;
	u32 nfcont;
	u32 nfcmmd;	/* [7:0] */
	u32 nfaddr; /* [7:0] */
	u32 nfdata; /* [7:0] */
	u32 nfmeccd0;
	u32 nfmeccd1;
	u32 nfseccd;
	u32 nfstat;
	u32 nfestat0;
	u32 nfestat1;
	u32 nfmecc0;
	u32 nfmecc1;
	u32 nfsecc;
	u32 nfsblk;
	u32 nfeblk;
};

struct mtd_info {
	struct nf_ctrl *nfctrl;

	int (*nf_read)(struct mtd_info *mtd, u32 offset, void *data, u32 *len);
	int (*nf_write)(struct mtd_info *mtd, u32 offset, void *data, u32 *len);
	int (*nf_erase)(struct mtd_info *mtd, u32 offset, u32 len);

	u32 writesize;
	u32 blocksize;
	u32 planesize;

	u32 page_shift;
	u32 block_shift;
};
static struct mtd_info mtdinfo;

static void nf_wait_ready(struct mtd_info *mtd)
{
	while(!(readb(&mtd->nfctrl->nfstat) & 0x1))
		;
}

static inline void cmd_ctrl(struct mtd_info *mtd, int cmd, int ctrl)
{
	if(ctrl & NF_CMD_CLE)
		writel(&mtd->nfctrl->nfcmmd, cmd & 0xFF);
	else
		writel(&mtd->nfctrl->nfaddr, cmd & 0xFF);
}

static void nf_command(struct mtd_info *mtd, int cmd, int column, int page)
{
	int ctrl = NF_CMD_CLE;

	/* check and set pointer before program */
	if(cmd == NAND_CMD_SEQIN) {
		int readcmd;
		if(column > mtd->writesize) {
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

	ctrl = NF_CMD_ALE;
	switch(cmd) {
		case NAND_CMD_ERASE1:
			/* followed by erase2 */
			cmd_ctrl(mtd, NAND_CMD_ERASE2, ctrl);
			return;
		case NAND_CMD_READID:
		case NAND_CMD_PAGEPROG0:
		case NAND_CMD_PAGEPROG1:
		case NAND_CMD_STATUS:
		case NAND_CMD_STATUS_MULTI:
			return;

		case NAND_CMD_RESET:
		default:
			break;
	}
	nf_wait_ready(mtd);
}

static u8 get_nfstatus(struct mtd_info *mtd)
{
	nf_command(mtd, NAND_CMD_STATUS, -1, -1);
	return readb(&mtd->nfctrl->nfdata);
}

/*
 * 0x60 -> block address -> 0xd0 -> read status -> i/o6=1? or RnB=1?
 * 	-> i/o0=0? -> error -> erase complete
 */
static int nf_erase(struct mtd_info *mtd, u32 page, u32 count)
{
	u8 stat;
	u32 block = page & ~((1 << mtd->block_shift) - 1);

	while(count--) {
		nf_command(mtd, NAND_CMD_ERASE1, 0x0, block++);

		while(!((stat = get_nfstatus(mtd)) & NF_STATUS_READY))
			;
		if(stat & NF_STATUS_PASS)
			puts(get_port_entry(0), "Error: erase not passed!\n");
	}

	return 0;
}

static void hw_setup(struct mtd_info *mtd)
{
	mtd->nfctrl = (struct nf_ctrl *)0x4E000000;
	writel(&mtd->nfctrl->nfconf, 1<<12 | 4<<8 | 1<<4);
	writel(&mtd->nfctrl->nfcont, 0<<1 | 1<<0);
	/*TODO*/
}

static void module_init(struct mtd_info *mtd)
{
	mtd->writesize = 512;
	mtd->blocksize = 512*32;
	mtd->planesize = 512*32*1024;
	mtd->page_shift = 9;
	mtd->block_shift = 14;
	mtd->nf_read = 0;
	mtd->nf_write = 0;
	mtd->nf_erase = nf_erase;
}

inline struct mtd_info *get_mtd_info(void)
{
	return &mtdinfo;
}

void nand_main(void)
{
	struct mtd_info *mtd = get_mtd_info();
#ifdef DEBUG_LEDS
	lights(5);
#endif
	hw_setup(mtd);
	module_init(mtd);

	while(1) {
		puts(get_port_entry(0), "Hello, the curel world...\n");
		putslong(0x12345678);
	}
}

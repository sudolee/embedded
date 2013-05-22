#ifndef _NAND_H_
#define _NAND_H_

#include "type.h"

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
//	u32 blocksize;
//	u32 planesize;

	u32 page_shift;
	u32 block_shift;
	u32 device_shift;
};

void nand_setup(void);
struct mtd_info *get_mtd_info(void);

#endif /* _NAND_H_ */

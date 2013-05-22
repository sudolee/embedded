#include "type.h"
#include "io.h"
#include "nand.h"
#include "print.h"
#include "str.h"
#include "uart.h"

#define MAGIC_SIZE 16
#define MAGIC_STR "Matti.Lee\0"

struct img_header {
	char magic[MAGIC_SIZE];

    u32 img_offset;
    u32 img_size;

    /* maybe some pad space */
};

static void check_magic(struct img_header *head)
{
	while(strcmp(MAGIC_STR, head->magic))
		mb();
}

#define DEFAULT_LOADADDR ((0xFFF - 1024) & ~0x7)
void loadbin(void)
{
	void (*go)(void);
	struct img_header *head = (struct img_header *)DEFAULT_LOADADDR;
	struct mtd_info *mtd = get_mtd_info();

	check_magic(head);
	mtd->nf_erase(mtd, 0x0, head->img_size);
	mtd->nf_write(mtd, 0x0, (void *)((u8*)head + head->img_offset), &head->img_size);

	go = (void *)((u8 *)head + head->img_offset);
	go();
}

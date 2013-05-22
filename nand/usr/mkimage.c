#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

typedef unsigned int u32;
#define MAGIC_SIZE 16
#define MAGIC_STR "Matti.Lee\0"

/*
 * +-----------------+ 
 * |     header      | m byte(s)
 * +-----------------+
 * |     image       | n byte(s)
 * +-----------------+
 */

struct header {
	char magic[MAGIC_SIZE];

	u32 img_offset;
	u32 img_size;

	/* maybe, some pad bytes */
};

static struct header *new_head(void)
{
	struct header *head;

	head = malloc(sizeof(struct header));
	if(!head)
		return NULL;

	strcpy(head->magic, MAGIC_STR);
	head->img_offset = sizeof(struct header);

	return head;
}

static void make_image(const char *in, const char *out)
{
	struct header *head;
	int fin, fout;
	int ret;
	char readbuf[512];


	head = new_head();
	if(!head){
		printf("%s: create head error.\n", __func__);
		exit(EXIT_FAILURE);
	}

	fin = open(in, O_RDONLY);
	if(fin == -1) {
		printf("%s: can't open %s\n", __func__, in);
		exit(EXIT_FAILURE);
	}

	/* fill image size area within header. */
	head->img_size = lseek(fin, 0, SEEK_END);
	printf("head->img_size = %d\n", head->img_size);
	lseek(fin, 0, SEEK_SET);

	fout = open(out, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if(fout == -1) {
		printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	ret = write(fout, (void*)head, sizeof(struct header));
	if(ret == -1) {
		printf("%s: write %s error.\n", __func__, out);
		exit(EXIT_FAILURE);
	}

	ret = 0;
	while(1) {
		ret = read(fin, readbuf, sizeof(readbuf));
		if(ret == -1) {
			printf("%s: read %s error.\n", __func__, in);
			exit(EXIT_FAILURE);
		} else if(ret == 0) {
			break;
		} else {
			ret = write(fout, readbuf, ret);
			if(ret == -1) {
				printf("%s: read %s error.\n", __func__, in);
				exit(EXIT_FAILURE);
			}
		}
	}

	close(fin);
	close(fout);
	printf("%s() done.\n", __func__);
}

static void usage(void)
{
	printf("Usage:  mkimage input-file output-file\n\n	\
Packed input path with a header, in format below:\n	\
+-----------------+\n	\
|     header      | %d byte(s)\n	\
+-----------------+\n	\
|   input file    | X byte(s)\n	\
+-----------------+\n", (int)sizeof(struct header));
}

int main(int argc, char **argv)
{
	if(argc == 3) {
		make_image(argv[1], argv[2]);
	} else {
		usage();
		exit(EXIT_FAILURE);
	}

	printf("Default download address: %#X\n", (0xFFF - 1024) & ~0x7);

	return 0;
}

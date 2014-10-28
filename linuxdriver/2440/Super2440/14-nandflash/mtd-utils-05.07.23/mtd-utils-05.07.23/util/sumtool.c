/*
 *  sumtool.c
 *
 *  Copyright (C) 2004 Zoltan Sogor <weth@inf.u-szeged.hu>,
 *                     Ferenc Havasi <havasi@inf.u-szeged.hu>,
 *                     Patrik Kluba <pajko@halom.u-szeged.hu>,
 *                     University of Szeged, Hungary
 *
 * $Id: sumtool.c,v 1.3 2005/02/17 14:55:07 hvr Exp $
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Overview:
 *   This is a utility to reorder nodes and insert inode summary information
 *   into JFFS3 image for faster mount time - specially on NAND.
 *
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <asm/types.h>
#include <dirent.h>
#include "jffs3.h"
#include <endian.h>
#include <byteswap.h>
#include <getopt.h>
#include "crc32.h"

#define CONFIG_JFFS3_SUMMARY
#define SUM_TOOL
#include "summary.h"


#define PAD(x) (((x)+3)&~3)

static const char *const app_name = "sumtool";

static struct jffs3_sum_info *sum_collected = NULL;/* summary info list */

static int verbose = 0;
static int padto = 0;						/* pad the output with 0xFF to the end of the final eraseblock */
static int add_cleanmarkers = 1;			/* add cleanmarker to output */
static int use_input_cleanmarker_size = 1;	/* use input file's cleanmarker size (default) */
static int found_cleanmarkers = 0;			/* cleanmarker found in input file */
static struct jffs3_unknown_node cleanmarker;
static int cleanmarker_size = sizeof(cleanmarker);
static const char *short_options = "o:i:e:hvVblnc:p";
static int erase_block_size = 65536;
int target_endian = __BYTE_ORDER;
static int out_fd = -1;
static int in_fd = -1;

static uint8_t *data_buffer = NULL; 	/* buffer for inodes */
static unsigned int data_ofs = 0;	 	/* inode buffer offset */

static uint8_t *file_buffer = NULL;		/* file buffer contains the actual erase block*/
static unsigned int file_ofs = 0;		/* position in the buffer */

static struct option long_options[] = {
	{"output", 1, NULL, 'o'},
	{"input", 1, NULL, 'i'},
	{"eraseblock", 1, NULL, 'e'},
	{"help", 0, NULL, 'h'},
	{"verbose", 0, NULL, 'v'},
	{"version", 0, NULL, 'V'},
	{"bigendian", 0, NULL, 'b'},
	{"littleendian", 0, NULL, 'l'},	
	{"no-cleanmarkers", 0, NULL, 'n'},
	{"cleanmarker", 1, NULL, 'c'},
	{"pad", 0, NULL, 'p'},
	{NULL, 0, NULL, 0}
};

static char *helptext =
	"Usage: sumtool [OPTIONS] -i inputfile -o outputfile\n"
	"Convert the input JFFS3 file to a SUM-ed JFFS3 file\n\n"
	"Options:\n"
	"  -e, --eraseblock=SIZE     Use erase block size SIZE (default: 64KiB)\n"
	"                            (usually 16KiB on NAND)\n"
	"  -c, --cleanmarker=SIZE    Size of cleanmarker (default 12).\n"
	"                            (usually 16 bytes on NAND, and will be set to\n"
	"                            this value if left at the default 12). Will be\n"
	"                            stored in OOB after each physical page composing\n"
	"                            a physical erase block.\n"
	"  -n, --no-cleanmarkers     Don't add a cleanmarker to every eraseblock\n"
	"  -o, --output=FILE         Output to FILE \n"
	"  -i, --input=FILE          Input from FILE \n"
	"  -b, --bigendian           Image is big endian\n"
	"  -l  --littleendian        Image is little endian\n"
	"  -h, --help                Display this help text\n"
	"  -v, --verbose             Verbose operation\n"
	"  -V, --version             Display version information\n"
	"  -p, --pad                 Pad the OUTPUT with 0xFF to the end of the final\n"
	"                            eraseblock\n\n";


static char *revtext = "$Revision: 1.3 $";

static unsigned char ffbuf[16] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static void verror_msg(const char *s, va_list p) {
	fflush(stdout);
	fprintf(stderr, "%s: ", app_name);
	vfprintf(stderr, s, p);
}

static void error_msg_and_die(const char *s, ...) {
	va_list p;

	va_start(p, s);
	verror_msg(s, p);
	va_end(p);
	putc('\n', stderr);
	exit(EXIT_FAILURE);
}

static void vperror_msg(const char *s, va_list p) {
	int err = errno;

	if (s == 0)
		s = "";
	verror_msg(s, p);
	if (*s)
		s = ": ";
	fprintf(stderr, "%s%s\n", s, strerror(err));
}

static void perror_msg_and_die(const char *s, ...) {
	va_list p;

	va_start(p, s);
	vperror_msg(s, p);
	va_end(p);
	exit(EXIT_FAILURE);
}



static void full_write(void *target_buff, const void *buf, int len);

void setup_cleanmarker() {

	cleanmarker.magic    = cpu_to_je16(JFFS3_MAGIC_BITMASK);
	cleanmarker.nodetype = cpu_to_je16(JFFS3_NODETYPE_CLEANMARKER);
	cleanmarker.totlen   = cpu_to_je32(cleanmarker_size);
	cleanmarker.hdr_crc  = cpu_to_je32(crc32(0, &cleanmarker, sizeof(struct jffs3_unknown_node)-4));
}

void process_options (int argc, char **argv){
	int opt,c;
	
	while ((opt = getopt_long(argc, argv, short_options, long_options, &c)) >= 0) 
	{
		switch (opt) 
		{
			case 'o':
				if (out_fd != -1) {
					error_msg_and_die("output filename specified more than once");
				}
				out_fd = open(optarg, O_CREAT | O_TRUNC | O_RDWR, 0644);
				if (out_fd == -1) {
					perror_msg_and_die("open output file");
				}
				break;
				
			case 'i':
				if (in_fd != -1) {
					error_msg_and_die("input filename specified more than once");
				}
				in_fd = open(optarg, O_RDONLY);
				if (in_fd == -1) {
					perror_msg_and_die("open input file");
				}
				break;
			case 'b':
				target_endian = __BIG_ENDIAN;
				break;
			case 'l':
				target_endian = __LITTLE_ENDIAN;
				break;	
			case 'h':
			case '?':
				error_msg_and_die(helptext);
	
			case 'v':
				verbose = 1;
				break;
	
			case 'V':
				error_msg_and_die("revision %.*s\n",
						(int) strlen(revtext) - 13, revtext + 11);
	
			case 'e': {
				char *next;
				unsigned units = 0;
				erase_block_size = strtol(optarg, &next, 0);
				if (!erase_block_size)
					error_msg_and_die("Unrecognisable erase size\n");
	
				if (*next) {
					if (!strcmp(next, "KiB")) {
						units = 1024;
					} else if (!strcmp(next, "MiB")) {
						units = 1024 * 1024;
					} else {
						error_msg_and_die("Unknown units in erasesize\n");
					}
				} else {
					if (erase_block_size < 0x1000)
						units = 1024;
					else
						units = 1;
				}
				erase_block_size *= units;
	
				/* If it's less than 8KiB, they're not allowed */
				if (erase_block_size < 0x2000) {
					fprintf(stderr, "Erase size 0x%x too small. Increasing to 8KiB minimum\n",
						erase_block_size);
					erase_block_size = 0x2000;
				}
				break;
			}

			case 'n':
				add_cleanmarkers = 0;
				break;
			case 'c':
				cleanmarker_size = strtol(optarg, NULL, 0);
			
				if (cleanmarker_size < sizeof(cleanmarker)) {
					error_msg_and_die("cleanmarker size must be >= 12");
				}
				if (cleanmarker_size >= erase_block_size) {
					error_msg_and_die("cleanmarker size must be < eraseblock size");
				}
				
				use_input_cleanmarker_size = 0;
				found_cleanmarkers = 1;
				setup_cleanmarker();
				
				break;
			case 'p':
				padto = 1;
				break;
			
		}
	}
}


void init_buffers() {
	
	data_buffer = malloc(erase_block_size);
	
	if (!data_buffer) {
		perror("out of memory");
		close (in_fd);
		close (out_fd);
		exit(1);
	}
		
	file_buffer = malloc(erase_block_size);
	
	if (!file_buffer) {
		perror("out of memory");
		close (in_fd);
		close (out_fd);
		exit(1);
	}
}

void init_sumlist(){
	
	sum_collected = (struct jffs3_sum_info *) malloc (sizeof(struct jffs3_sum_info));
	
	if (!sum_collected)
		error_msg_and_die("Can't allocate memory for jffs3_sum_info!\n");
	
	sum_collected->sum_list = NULL;
	sum_collected->sum_num = 0;
	sum_collected->sum_size = 0; 

}

void clean_buffers() {
	
	if (data_buffer) 
		free(data_buffer);
	if (file_buffer)
		free(file_buffer);
}

void clean_sumlist() {
	
	union jffs3_sum_mem *temp;
	
	if(sum_collected){
		
		while(sum_collected->sum_list){
			temp = sum_collected->sum_list;
			sum_collected->sum_list = sum_collected->sum_list->u.next;
			free(temp);
			sum_collected->sum_num--;
		}
		
		if(sum_collected->sum_num != 0)
			printf("Ooops, something wrong happened! sum_num != 0, but sum_list = null ???");
		
		free(sum_collected);
	}	
}

int load_next_block() {
	
	int ret;
	ret = read(in_fd, file_buffer, erase_block_size);
	file_ofs = 0;
	
	if(verbose)
		printf("Load next block : %d bytes read\n",ret);
	
	return ret;
}

void write_buff_to_file() {
	
	int ret;
	int len = data_ofs;
	
	uint8_t *buf = NULL;
	
	buf = data_buffer;
	while (len > 0) {
		ret = write(out_fd, buf, len);
	
		if (ret < 0)
			perror_msg_and_die("write");

		if (ret == 0)
			perror_msg_and_die("write returned zero");
	
		len -= ret;
		buf += ret;
	}
	
	data_ofs = 0;
}

void dump_sum_records() {
	
    struct jffs3_summary_node isum;
    union jffs3_sum_mem *temp;
	jint32_t offset;
	jint32_t *wpage;
	uint8_t *tempptr;
	int datasize;
	int infosize;
	int padsize;
	jint32_t magic = cpu_to_je32(JFFS3_SUM_MAGIC);
	
	if (!sum_collected->sum_num || !sum_collected->sum_list) 
		return; 
	
	datasize = sum_collected->sum_size + sizeof(struct jffs3_sum_marker);
	infosize = sizeof(struct jffs3_summary_node) + datasize;
	padsize = erase_block_size - data_ofs - infosize;
	infosize += padsize; datasize += padsize;
	offset = cpu_to_je32(data_ofs);
	
	jint32_t *tpage = (jint32_t *) malloc(datasize);
	
	if(!tpage)
		error_msg_and_die("Can't allocate memory to dump summary information!\n");
	
	memset(tpage, 0xff, datasize);
	memset(&isum, 0, sizeof(isum));
	
	isum.magic = cpu_to_je16(JFFS3_MAGIC_BITMASK);
	isum.nodetype = cpu_to_je16(JFFS3_NODETYPE_SUMMARY);
	isum.totlen = cpu_to_je32(infosize);
	isum.hdr_crc = cpu_to_je32(crc32(0, &isum, sizeof(struct jffs3_unknown_node) - 4));
        isum.padded = cpu_to_je32(0);
		
	if (add_cleanmarkers && found_cleanmarkers) {
		isum.cln_mkr = cpu_to_je32(cleanmarker_size);	
	}
	else{
		isum.cln_mkr = cpu_to_je32(0);
	}
	
	isum.sum_num = cpu_to_je16(sum_collected->sum_num);
	wpage = tpage;
	
		
	while (sum_collected->sum_num) {
		

		switch(je16_to_cpu(sum_collected->sum_list->u.nodetype)){
			
			case JFFS3_NODETYPE_INODE : {
				jint16_t *temp16ptr = (jint16_t *)wpage;
				
				*(temp16ptr++) = sum_collected->sum_list->i.nodetype;
				wpage = (jint32_t *) temp16ptr;
				
				*(wpage++) = sum_collected->sum_list->i.inode;
				*(wpage++) = sum_collected->sum_list->i.version;
				*(wpage++) = sum_collected->sum_list->i.offset;
				*(wpage++) = sum_collected->sum_list->i.totlen;
				break;
			}
			
			case JFFS3_NODETYPE_DIRENT : {
				jint16_t *temp16ptr = (jint16_t *) wpage;
				uint8_t *temp8ptr = NULL;
				
				*(temp16ptr++) = sum_collected->sum_list->d.nodetype;
				wpage = (jint32_t *) temp16ptr;
				
				*(wpage++) = sum_collected->sum_list->d.totlen;
				*(wpage++) = sum_collected->sum_list->d.offset;
				*(wpage++) = sum_collected->sum_list->d.pino;
				*(wpage++) = sum_collected->sum_list->d.version;
				*(wpage++) = sum_collected->sum_list->d.ino;
				
				temp8ptr = (uint8_t *) wpage;
				*(temp8ptr++) = sum_collected->sum_list->d.nsize;
				*(temp8ptr++) = sum_collected->sum_list->d.type;
				
				memcpy(temp8ptr,sum_collected->sum_list->d.name,sum_collected->sum_list->d.nsize);
				temp8ptr += sum_collected->sum_list->d.nsize;
				wpage = (jint32_t *) temp8ptr;
				
				break;
			}
			
			default : {
				printf("This is not good for me!!!!\n");
			}
		}
		
		temp = sum_collected->sum_list;
		sum_collected->sum_list = sum_collected->sum_list->u.next;
		free(temp);
		
		sum_collected->sum_num--;
	}
	
	sum_collected->sum_size = 0;
	
	tempptr = (uint8_t *) wpage;
	tempptr += padsize;
	wpage = (jint32_t *) tempptr;
	
	*(wpage++) = offset;
	*(wpage++) = cpu_to_je32(erase_block_size);
	*(wpage++) = magic;
	isum.sum_crc = cpu_to_je32(crc32(0, tpage, datasize));
	isum.node_crc = cpu_to_je32(crc32(0, &isum, sizeof(isum) - 8));
	
	full_write(data_buffer + data_ofs, &isum, sizeof(isum));
	full_write(data_buffer + data_ofs, tpage, datasize);
	
	free(tpage);

}

static void full_write(void *target_buff, const void *buf, int len) {
	
	memcpy(target_buff, buf, len);
	data_ofs += len;
}

static void pad(int req) {
	while (req) {
		if (req > sizeof(ffbuf)) {
			full_write(data_buffer + data_ofs, ffbuf, sizeof(ffbuf));
			req -= sizeof(ffbuf);
		} else {
			full_write(data_buffer + data_ofs, ffbuf, req);
			req = 0;
		}
	}
} 

static inline void padword() {
	
	if (data_ofs % 4) {
		full_write(data_buffer + data_ofs, ffbuf, 4 - (data_ofs % 4));
	}
} 


static inline void pad_block_if_less_than(int req,int plus) {
	
    int datasize = req + plus + sum_collected->sum_size + sizeof(struct jffs3_summary_node) + 8;
    datasize += (4 - (datasize % 4)) % 4;
	
    if (data_ofs + req > erase_block_size - datasize) {
        dump_sum_records();
		write_buff_to_file();
    }
		
	if (add_cleanmarkers && found_cleanmarkers) {
		if (!data_ofs) {
			full_write(data_buffer, &cleanmarker, sizeof(cleanmarker));
			pad(cleanmarker_size - sizeof(cleanmarker));
			padword();
		}
	}
}

void flush_buffers() {
	
	if ((add_cleanmarkers == 1) && (found_cleanmarkers == 1)) { /* CLEANMARKER */
		if (data_ofs != cleanmarker_size) {	/* INODE BUFFER */
			
		    int datasize = sum_collected->sum_size + sizeof(struct jffs3_summary_node) + 8;
		    datasize += (4 - (datasize % 4)) % 4;
			
			/* If we have a full inode buffer, then write out inode and summary data  */
		    if (data_ofs + sizeof(struct jffs3_raw_inode) + 2*JFFS3_MIN_DATA_LEN > erase_block_size - datasize) {
		        dump_sum_records();
				write_buff_to_file();
		    }
			/* else just write out inode data */
			else{
				if(padto)
					pad(erase_block_size - data_ofs);
				write_buff_to_file();
			}
		}
	}
	else { /* NO CLEANMARKER */
		if (data_ofs != 0) { /* INODE BUFFER */
			
		    int datasize = sum_collected->sum_size + sizeof(struct jffs3_summary_node) + 8;
		    datasize += (4 - (datasize % 4)) % 4;
			
			/* If we have a full inode buffer, then write out inode and summary data */
		    if (data_ofs + sizeof(struct jffs3_raw_inode) + 2*JFFS3_MIN_DATA_LEN > erase_block_size - datasize) {
		        dump_sum_records();
				write_buff_to_file();
		    }
			/* Else just write out inode data */
			else{
				if(padto)
					pad(erase_block_size - data_ofs);
				write_buff_to_file();
			}
		}
	}
}

int add_sum_mem(union jffs3_sum_mem *item) {
	
	union jffs3_sum_mem *walk;
	
	if (!sum_collected->sum_list) {
		sum_collected->sum_list = (union jffs3_sum_mem *) item;
	} 
	else {
		walk = sum_collected->sum_list;
		
		while (walk->u.next) {
			walk = walk->u.next;
		}
		walk->u.next = (union jffs3_sum_mem *) item;
	}
	switch (je16_to_cpu(item->u.nodetype)) {
    	case JFFS3_NODETYPE_INODE:
			sum_collected->sum_size += JFFS3_SUMMARY_INODE_SIZE;
			sum_collected->sum_num++;
			break;
		
    	case JFFS3_NODETYPE_DIRENT:
			sum_collected->sum_size += JFFS3_SUMMARY_DIRENT_SIZE(item->d.nsize);
			sum_collected->sum_num++;
			break;
		
	    default:
			error_msg_and_die("__jffs3_add_sum_mem(): UNKNOWN node type %d\n", je16_to_cpu(item->u.nodetype));
	}
	return 0;
}

void add_sum_inode_mem(union jffs3_node_union *node) {
	
	struct jffs3_sum_inode_mem *temp = (struct jffs3_sum_inode_mem *) malloc(sizeof(struct jffs3_sum_inode_mem));
	
	if (!temp)
		error_msg_and_die("Can't allocate memory for summary information!\n");
	
	temp->nodetype = node->i.nodetype;
	temp->inode = node->i.ino;
	temp->version = node->i.version;
	temp->offset = cpu_to_je32(data_ofs); 
	temp->totlen = node->i.totlen;
	temp->next = NULL;

	add_sum_mem((union jffs3_sum_mem *) temp);
	
}

void add_sum_dirent_mem(union jffs3_node_union *node) {
	
	struct jffs3_sum_dirent_mem *temp = (struct jffs3_sum_dirent_mem *) 
			malloc(sizeof(struct jffs3_sum_dirent_mem) + node->d.nsize);
	
	if (!temp)
		error_msg_and_die("Can't allocate memory for summary information!\n");
	
	temp->nodetype = node->d.nodetype;
	temp->totlen = node->d.totlen;
	temp->offset = cpu_to_je32(data_ofs);
	temp->pino = node->d.pino;
	temp->version = node->d.version;
	temp->ino = node->d.ino;
	temp->nsize = node->d.nsize;
	temp->type = node->d.type;
	temp->next = NULL;
	
	memcpy(temp->name,node->d.name,node->d.nsize);

	add_sum_mem((union jffs3_sum_mem *) temp);
	
}

void write_dirent_to_buff(union jffs3_node_union *node) {
	
	pad_block_if_less_than(je32_to_cpu (node->d.totlen),JFFS3_SUMMARY_DIRENT_SIZE(node->d.nsize));
	add_sum_dirent_mem(node);
	full_write(data_buffer + data_ofs, &(node->d), je32_to_cpu (node->d.totlen));
	padword();	
}


void write_inode_to_buff(union jffs3_node_union *node) {
	
	pad_block_if_less_than(je32_to_cpu (node->i.totlen),JFFS3_SUMMARY_INODE_SIZE);  
	add_sum_inode_mem(node);	/* Add inode summary mem to summary list */
	full_write(data_buffer + data_ofs, &(node->i), je32_to_cpu (node->i.totlen));	/* Write out the inode to inode_buffer */
	padword();
	
}


void create_summed_image(int inp_size) {
	uint8_t		*p = file_buffer;
	union jffs3_node_union 	*node;
	uint32_t	crc;
	uint16_t	type;
	int		bitchbitmask = 0;
	int		obsolete;
	
	char	name[256];
	
	while ( p < (file_buffer + inp_size)) {
		
		node = (union jffs3_node_union *) p;
		
		/* Skip empty space */
		if (je16_to_cpu (node->u.magic) == 0xFFFF && je16_to_cpu (node->u.nodetype) == 0xFFFF) {
			p += 4;
			continue;
		}
		
		if (je16_to_cpu (node->u.magic) != JFFS3_MAGIC_BITMASK)	{
			if (!bitchbitmask++)
    			    printf ("Wrong bitmask  at  0x%08x, 0x%04x\n", p - file_buffer, je16_to_cpu (node->u.magic));
			p += 4;
			continue;
		}
		
		bitchbitmask = 0;
		
		type = je16_to_cpu(node->u.nodetype);
		if ((type & JFFS3_NODE_ACCURATE) != JFFS3_NODE_ACCURATE) {
			obsolete = 1;
			type |= JFFS3_NODE_ACCURATE;
		} else
			obsolete = 0;
		
		node->u.nodetype = cpu_to_je16(type);
	    
		crc = crc32 (0, node, sizeof (struct jffs3_unknown_node) - 4);
		if (crc != je32_to_cpu (node->u.hdr_crc)) {
			printf ("Wrong hdr_crc  at  0x%08x, 0x%08x instead of 0x%08x\n", p - file_buffer, je32_to_cpu (node->u.hdr_crc), crc);
			p += 4;
			continue;
		}
		
		switch(je16_to_cpu(node->u.nodetype)) {
		
			case JFFS3_NODETYPE_INODE:
				if(verbose)
					printf ("%8s Inode      node at 0x%08x, totlen 0x%08x, #ino  %5d, version %5d, isize %8d, csize %8d, dsize %8d, offset %8d\n",
						obsolete ? "Obsolete" : "",
						p - file_buffer, je32_to_cpu (node->i.totlen), je32_to_cpu (node->i.ino),
						je32_to_cpu ( node->i.version), je32_to_cpu (node->i.isize), 
						je32_to_cpu (node->i.csize), je32_to_cpu (node->i.dsize), je32_to_cpu (node->i.offset));
	
				crc = crc32 (0, node, sizeof (struct jffs3_raw_inode) - 8);
				if (crc != je32_to_cpu (node->i.node_crc)) {
					printf ("Wrong node_crc at  0x%08x, 0x%08x instead of 0x%08x\n", p - file_buffer, je32_to_cpu (node->i.node_crc), crc);
					p += PAD(je32_to_cpu (node->i.totlen));
					continue;
				}
				
				crc = crc32(0, p + sizeof (struct jffs3_raw_inode), je32_to_cpu(node->i.csize));
				if (crc != je32_to_cpu(node->i.data_crc)) {
					printf ("Wrong data_crc at  0x%08x, 0x%08x instead of 0x%08x\n", p - file_buffer, je32_to_cpu (node->i.data_crc), crc);
					p += PAD(je32_to_cpu (node->i.totlen));
					continue;
				}
				
				write_inode_to_buff(node);
				
				p += PAD(je32_to_cpu (node->i.totlen));
				break;
				
			case JFFS3_NODETYPE_DIRENT:
				memcpy (name, node->d.name, node->d.nsize);
				name [node->d.nsize] = 0x0;
			
				if(verbose)
					printf ("%8s Dirent     node at 0x%08x, totlen 0x%08x, #pino %5d, version %5d, #ino  %8d, nsize %8d, name %s\n",
						obsolete ? "Obsolete" : "",
						p - file_buffer, je32_to_cpu (node->d.totlen), je32_to_cpu (node->d.pino),
						je32_to_cpu ( node->d.version), je32_to_cpu (node->d.ino), 
						node->d.nsize, name);
	
				crc = crc32 (0, node, sizeof (struct jffs3_raw_dirent) - 8);
				if (crc != je32_to_cpu (node->d.node_crc)) {
					printf ("Wrong node_crc at  0x%08x, 0x%08x instead of 0x%08x\n", p - file_buffer, je32_to_cpu (node->d.node_crc), crc);
					p += PAD(je32_to_cpu (node->d.totlen));
					continue;
				}
				
				crc = crc32(0, p + sizeof (struct jffs3_raw_dirent), node->d.nsize);
				if (crc != je32_to_cpu(node->d.name_crc)) {
					printf ("Wrong name_crc at  0x%08x, 0x%08x instead of 0x%08x\n", p - file_buffer, je32_to_cpu (node->d.name_crc), crc);
					p += PAD(je32_to_cpu (node->d.totlen));
					continue;
				}
	
				write_dirent_to_buff(node);
				
				p += PAD(je32_to_cpu (node->d.totlen));						
				break;
		
			case JFFS3_NODETYPE_CLEANMARKER:
				if (verbose) {
					printf ("%8s Cleanmarker     at 0x%08x, totlen 0x%08x\n", 
						obsolete ? "Obsolete" : "",
						p - file_buffer, je32_to_cpu (node->u.totlen));
				}
				
				if(!found_cleanmarkers){
					found_cleanmarkers = 1;
					
					if(add_cleanmarkers == 1 && use_input_cleanmarker_size == 1){
						cleanmarker_size = je32_to_cpu (node->u.totlen);
						setup_cleanmarker();
					}
				}			
				
				p += PAD(je32_to_cpu (node->u.totlen));						
				break;
		
			case JFFS3_NODETYPE_PADDING:
				if (verbose) {
					printf ("%8s Padding    node at 0x%08x, totlen 0x%08x\n", 
						obsolete ? "Obsolete" : "",
						p - file_buffer, je32_to_cpu (node->u.totlen));
				}		
				p += PAD(je32_to_cpu (node->u.totlen));						
				break;
				
			case 0xffff:
				p += 4;
				break;
				
			default:	
				if (verbose) {
					printf ("%8s Unknown    node at 0x%08x, totlen 0x%08x\n", 
						obsolete ? "Obsolete" : "",
						p - file_buffer, je32_to_cpu (node->u.totlen));
				}
				
				//write_unknown_to_buff(node);
				
				p += PAD(je32_to_cpu (node->u.totlen));						
		}	
	}
}

int main(int argc, char **argv) {
	
	int ret;
	
	process_options(argc,argv);
	
	if ((in_fd == -1) || (out_fd == -1))	{
		
		if(in_fd != -1)
			close(in_fd);
		if(out_fd != -1)
			close(out_fd);
		
		fprintf(stderr,helptext);
		error_msg_and_die("You must specify input and output files!\n");
	}
	
	init_buffers();
	init_sumlist();
	
	while ((ret = load_next_block())) {
		create_summed_image(ret);	
	}

	flush_buffers();
	clean_buffers();
	clean_sumlist();
	
	if (in_fd != -1)
		close(in_fd);
	if (out_fd != -1)
		close(out_fd);
	
	return 0;
}

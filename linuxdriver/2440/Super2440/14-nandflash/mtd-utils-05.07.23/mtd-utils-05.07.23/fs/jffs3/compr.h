/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2004 Ferenc Havasi <havasi@inf.u-szeged.hu>,
 *                    University of Szeged, Hungary
 *
 * For licensing information, see the file 'LICENCE' in the
 * jffs3 directory.
 *
 * JFFS2 Id: compr.h,v 1.6 2004/07/16 15:17:57 dwmw2  Exp
 * $Id: compr.h,v 3.5 2005/01/05 16:18:59 dedekind Exp $
 *
 */

#ifndef __JFFS3_COMPR_H__
#define __JFFS3_COMPR_H__

#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include "jffs3.h"
#include "nodelist.h"

#define JFFS3_RUBINMIPS_PRIORITY 10
#define JFFS3_DYNRUBIN_PRIORITY  20
#define JFFS3_LZARI_PRIORITY     30
#define JFFS3_LZO_PRIORITY       40
#define JFFS3_RTIME_PRIORITY     50
#define JFFS3_ZLIB_PRIORITY      60

#define JFFS3_RUBINMIPS_DISABLED /* RUBINs will be used only */
#define JFFS3_DYNRUBIN_DISABLED  /*        for decompression */

#define JFFS3_COMPR_MODE_NONE       0
#define JFFS3_COMPR_MODE_PRIORITY   1
#define JFFS3_COMPR_MODE_SIZE       2

void jffs3_set_compression_mode(int mode);
int jffs3_get_compression_mode(void);

struct jffs3_compressor {
        struct list_head list;
        int priority;              /* used by prirority comr. mode */
        char *name;
        char compr;                /* JFFS3_COMPR_XXX */
        int (*compress)(unsigned char *data_in, unsigned char *cpage_out,
                        uint32_t *srclen, uint32_t *destlen, void *model);
        int (*decompress)(unsigned char *cdata_in, unsigned char *data_out,
                        uint32_t cdatalen, uint32_t datalen, void *model);
        int usecount;
        int disabled;              /* if seted the compressor won't compress */
        unsigned char *compr_buf;  /* used by size compr. mode */
        uint32_t compr_buf_size;   /* used by size compr. mode */
        uint32_t stat_compr_orig_size;
        uint32_t stat_compr_new_size;
        uint32_t stat_compr_blocks;
        uint32_t stat_decompr_blocks;
};

int jffs3_register_compressor(struct jffs3_compressor *comp);
int jffs3_unregister_compressor(struct jffs3_compressor *comp);

int jffs3_compressors_init(void);
int jffs3_compressors_exit(void);

uint16_t jffs3_compress(struct jffs3_sb_info *c, struct jffs3_inode_info *f,
                             unsigned char *data_in, unsigned char **cpage_out,
                             uint32_t *datalen, uint32_t *cdatalen);

int jffs3_decompress(struct jffs3_sb_info *c, struct jffs3_inode_info *f,
                     uint16_t comprtype, unsigned char *cdata_in,
                     unsigned char *data_out, uint32_t cdatalen, uint32_t datalen);

void jffs3_free_comprbuf(unsigned char *comprbuf, unsigned char *orig);

#ifdef CONFIG_JFFS3_PROC
int jffs3_enable_compressor_name(const char *name);
int jffs3_disable_compressor_name(const char *name);
int jffs3_set_compression_mode_name(const char *mode_name);
char *jffs3_get_compression_mode_name(void);
int jffs3_set_compressor_priority(const char *mode_name, int priority);
char *jffs3_list_compressors(void);
char *jffs3_stats(void);
#endif

/* Compressor modules */
/* These functions will be called by jffs3_compressors_init/exit */

#ifdef CONFIG_JFFS3_RUBIN
int jffs3_rubinmips_init(void);
void jffs3_rubinmips_exit(void);
int jffs3_dynrubin_init(void);
void jffs3_dynrubin_exit(void);
#endif
#ifdef CONFIG_JFFS3_RTIME
int jffs3_rtime_init(void);
void jffs3_rtime_exit(void);
#endif
#ifdef CONFIG_JFFS3_ZLIB
int jffs3_zlib_init(void);
void jffs3_zlib_exit(void);
#endif
#ifdef CONFIG_JFFS3_LZARI
int jffs3_lzari_init(void);
void jffs3_lzari_exit(void);
#endif
#ifdef CONFIG_JFFS3_LZO
int jffs3_lzo_init(void);
void jffs3_lzo_exit(void);
#endif

#endif /* __JFFS3_COMPR_H__ */

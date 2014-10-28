/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 * Created by Arjan van de Ven <arjanv@redhat.com>
 *
 * Copyright (C) 2004 Ferenc Havasi <havasi@inf.u-szeged.hu>,
 *                    University of Szeged, Hungary
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: compr.c,v 1.42 2004/08/07 21:56:08 dwmw2  Exp
 * $Id: compr.c,v 3.6 2005/01/05 16:18:59 dedekind Exp $
 *
 */

#include "compr.h"
#include "nodelist.h"

static spinlock_t jffs3_compressor_list_lock = SPIN_LOCK_UNLOCKED;

/* Available compressors are on this list */
static LIST_HEAD(jffs3_compressor_list);

/* Actual compression mode */
static int jffs3_compression_mode = JFFS3_COMPR_MODE_PRIORITY;

void jffs3_set_compression_mode(int mode)
{
        jffs3_compression_mode = mode;
}

int jffs3_get_compression_mode(void)
{
        return jffs3_compression_mode;
}

/* Statistics for blocks stored without compression */
static uint32_t none_stat_compr_blocks=0,none_stat_decompr_blocks=0,none_stat_compr_size=0;

/* jffs3_compress:
 * @data: Pointer to uncompressed data
 * @cdata: Pointer to returned pointer to buffer for compressed data
 * @datalen: On entry, holds the amount of data available for compression.
 *	On exit, expected to hold the amount of data actually compressed.
 * @cdatalen: On entry, holds the amount of space available for compressed
 *	data. On exit, expected to hold the actual size of the compressed
 *	data.
 *
 * Returns: Lower byte to be stored with data indicating compression type used.
 * Zero is used to show that the data could not be compressed - the
 * compressed version was actually larger than the original.
 * Upper byte will be used later. (soon)
 *
 * If the cdata buffer isn't large enough to hold all the uncompressed data,
 * jffs3_compress should compress as much as will fit, and should set
 * *datalen accordingly to show the amount of data which were compressed.
 */
uint16_t jffs3_compress(struct jffs3_sb_info *c, struct jffs3_inode_info *f,
			     unsigned char *data_in, unsigned char **cpage_out,
			     uint32_t *datalen, uint32_t *cdatalen)
{
	int ret = JFFS3_COMPR_NONE;
        int compr_ret;
        struct jffs3_compressor *this, *best=NULL;
        unsigned char *output_buf = NULL, *tmp_buf;
        uint32_t orig_slen, orig_dlen;
        uint32_t best_slen=0, best_dlen=0;

        switch (jffs3_compression_mode) {
        case JFFS3_COMPR_MODE_NONE:
                break;
        case JFFS3_COMPR_MODE_PRIORITY:
                output_buf = kmalloc(*cdatalen,GFP_KERNEL);
                if (!output_buf) {
                        WARNING_MSG("No memory for compressor allocation. Compression failed.\n");
                        goto out;
                }
                orig_slen = *datalen;
                orig_dlen = *cdatalen;
                spin_lock(&jffs3_compressor_list_lock);
                list_for_each_entry(this, &jffs3_compressor_list, list) {
                        /* Skip decompress-only backwards-compatibility and disabled modules */
                        if ((!this->compress)||(this->disabled))
                                continue;

                        this->usecount++;
                        spin_unlock(&jffs3_compressor_list_lock);
                        *datalen  = orig_slen;
                        *cdatalen = orig_dlen;
                        compr_ret = this->compress(data_in, output_buf, datalen, cdatalen, NULL);
                        spin_lock(&jffs3_compressor_list_lock);
                        this->usecount--;
                        if (!compr_ret) {
                                ret = this->compr;
                                this->stat_compr_blocks++;
                                this->stat_compr_orig_size += *datalen;
                                this->stat_compr_new_size  += *cdatalen;
                                break;
                        }
                }
                spin_unlock(&jffs3_compressor_list_lock);
                if (ret == JFFS3_COMPR_NONE) kfree(output_buf);
                break;
        case JFFS3_COMPR_MODE_SIZE:
                orig_slen = *datalen;
                orig_dlen = *cdatalen;
                spin_lock(&jffs3_compressor_list_lock);
                list_for_each_entry(this, &jffs3_compressor_list, list) {
                        /* Skip decompress-only backwards-compatibility and disabled modules */
                        if ((!this->compress)||(this->disabled))
                                continue;
                        /* Allocating memory for output buffer if necessary */
                        if ((this->compr_buf_size<orig_dlen)&&(this->compr_buf)) {
                                spin_unlock(&jffs3_compressor_list_lock);
                                kfree(this->compr_buf);
                                spin_lock(&jffs3_compressor_list_lock);
                                this->compr_buf_size=0;
                                this->compr_buf=NULL;
                        }
                        if (!this->compr_buf) {
                                spin_unlock(&jffs3_compressor_list_lock);
                                tmp_buf = kmalloc(orig_dlen,GFP_KERNEL);
                                spin_lock(&jffs3_compressor_list_lock);
                                if (!tmp_buf) {
                                        WARNING_MSG("No memory for compressor allocation. (%d bytes)\n", orig_dlen);
                                        continue;
                                }
                                else {
                                        this->compr_buf = tmp_buf;
                                        this->compr_buf_size = orig_dlen;
                                }
                        }
                        this->usecount++;
                        spin_unlock(&jffs3_compressor_list_lock);
                        *datalen  = orig_slen;
                        *cdatalen = orig_dlen;
                        compr_ret = this->compress(data_in, this->compr_buf, datalen, cdatalen, NULL);
                        spin_lock(&jffs3_compressor_list_lock);
                        this->usecount--;
                        if (!compr_ret) {
                                if ((!best_dlen)||(best_dlen>*cdatalen)) {
                                        best_dlen = *cdatalen;
                                        best_slen = *datalen;
                                        best = this;
                                }
                        }
                }
                if (best_dlen) {
                        *cdatalen = best_dlen;
                        *datalen  = best_slen;
                        output_buf = best->compr_buf;
                        best->compr_buf = NULL;
                        best->compr_buf_size = 0;
                        best->stat_compr_blocks++;
                        best->stat_compr_orig_size += best_slen;
                        best->stat_compr_new_size  += best_dlen;
                        ret = best->compr;
                }
                spin_unlock(&jffs3_compressor_list_lock);
                break;
        default:
                ERROR_MSG("Unknow compression mode.\n");
        }
 out:
        if (ret == JFFS3_COMPR_NONE) {
	        *cpage_out = data_in;
	        *datalen = *cdatalen;
                none_stat_compr_blocks++;
                none_stat_compr_size += *datalen;
        }
        else {
                *cpage_out = output_buf;
        }
	return ret;
}

int jffs3_decompress(struct jffs3_sb_info *c, struct jffs3_inode_info *f,
		     uint16_t comprtype, unsigned char *cdata_in,
		     unsigned char *data_out, uint32_t cdatalen, uint32_t datalen)
{
        struct jffs3_compressor *this;
        int ret;

	/* Older code had a bug where it would write non-zero 'usercompr'
	   fields. Deal with it. */
	if ((comprtype & 0xff) <= JFFS3_COMPR_ZLIB)
		comprtype &= 0xff;

	switch (comprtype & 0xff) {
	case JFFS3_COMPR_NONE:
		/* This should be special-cased elsewhere, but we might as well deal with it */
		memcpy(data_out, cdata_in, datalen);
                none_stat_decompr_blocks++;
		break;
	case JFFS3_COMPR_ZERO:
		memset(data_out, 0, datalen);
		break;
	default:
                spin_lock(&jffs3_compressor_list_lock);
                list_for_each_entry(this, &jffs3_compressor_list, list) {
                        if (comprtype == this->compr) {
                                this->usecount++;
                                spin_unlock(&jffs3_compressor_list_lock);
                                ret = this->decompress(cdata_in, data_out, cdatalen, datalen, NULL);
                                spin_lock(&jffs3_compressor_list_lock);
                                if (ret) {
                                        WARNING_MSG("Decompressor \"%s\" returned %d\n", this->name, ret);
                                }
                                else {
                                        this->stat_decompr_blocks++;
                                }
                                this->usecount--;
                                spin_unlock(&jffs3_compressor_list_lock);
                                return ret;
                        }
                }
		WARNING_MSG("Compression type 0x%02x not available.\n", comprtype);
                spin_unlock(&jffs3_compressor_list_lock);
		return -EIO;
	}
	return 0;
}

int jffs3_register_compressor(struct jffs3_compressor *comp)
{
        struct jffs3_compressor *this;

        if (!comp->name) {
                WARNING_MSG("NULL compressor name at registering JFFS3 compressor. Failed.\n");
                return -1;
        }
        comp->compr_buf_size=0;
        comp->compr_buf=NULL;
        comp->usecount=0;
        comp->stat_compr_orig_size=0;
        comp->stat_compr_new_size=0;
        comp->stat_compr_blocks=0;
        comp->stat_decompr_blocks=0;
        DBG_COMPR(1, "Registering JFFS3 compressor \"%s\"\n", comp->name);

        spin_lock(&jffs3_compressor_list_lock);

        list_for_each_entry(this, &jffs3_compressor_list, list) {
                if (this->priority < comp->priority) {
                        list_add(&comp->list, this->list.prev);
                        goto out;
                }
        }
        list_add_tail(&comp->list, &jffs3_compressor_list);
out:
        if (DEBUG2) {
		list_for_each_entry(this, &jffs3_compressor_list, list)
			DBG_COMPR(1, "Compressor \"%s\", prio %d\n", this->name, this->priority);
	}

        spin_unlock(&jffs3_compressor_list_lock);

        return 0;
}

int jffs3_unregister_compressor(struct jffs3_compressor *comp)
{
        DBG_COMPR(1, "Unregistering JFFS3 compressor \"%s\"\n", comp->name);

        spin_lock(&jffs3_compressor_list_lock);

        if (comp->usecount) {
                spin_unlock(&jffs3_compressor_list_lock);
                WARNING_MSG("Compressor modul is in use. Unregister failed.\n");
                return -1;
        }
        list_del(&comp->list);

        if (DEBUG2) {
	        struct jffs3_compressor *this;
		list_for_each_entry(this, &jffs3_compressor_list, list)
			DBG_COMPR(2, "Compressor \"%s\", prio %d\n", this->name, this->priority);
        }
        spin_unlock(&jffs3_compressor_list_lock);
        return 0;
}

#ifdef CONFIG_JFFS3_PROC

#define JFFS3_STAT_BUF_SIZE 16000

char *jffs3_list_compressors(void)
{
        struct jffs3_compressor *this;
        char *buf, *act_buf;

        act_buf = buf = kmalloc(JFFS3_STAT_BUF_SIZE,GFP_KERNEL);
        list_for_each_entry(this, &jffs3_compressor_list, list) {
                act_buf += sprintf(act_buf, "%10s priority:%d ", this->name, this->priority);
                if ((this->disabled)||(!this->compress))
                        act_buf += sprintf(act_buf,"disabled");
                else
                        act_buf += sprintf(act_buf,"enabled");
                act_buf += sprintf(act_buf,"\n");
        }
        return buf;
}

char *jffs3_stats(void)
{
        struct jffs3_compressor *this;
        char *buf, *act_buf;

        act_buf = buf = kmalloc(JFFS3_STAT_BUF_SIZE,GFP_KERNEL);

        act_buf += sprintf(act_buf,"JFFS3 compressor statistics:\n");
        act_buf += sprintf(act_buf,"%10s   ","none");
        act_buf += sprintf(act_buf,"compr: %d blocks (%d)  decompr: %d blocks\n", none_stat_compr_blocks,
                           none_stat_compr_size, none_stat_decompr_blocks);
        spin_lock(&jffs3_compressor_list_lock);
        list_for_each_entry(this, &jffs3_compressor_list, list) {
                act_buf += sprintf(act_buf,"%10s ",this->name);
                if ((this->disabled)||(!this->compress))
                        act_buf += sprintf(act_buf,"- ");
                else
                        act_buf += sprintf(act_buf,"+ ");
                act_buf += sprintf(act_buf,"compr: %d blocks (%d/%d)  decompr: %d blocks ", this->stat_compr_blocks,
                                   this->stat_compr_new_size, this->stat_compr_orig_size,
                                   this->stat_decompr_blocks);
                act_buf += sprintf(act_buf,"\n");
        }
        spin_unlock(&jffs3_compressor_list_lock);

        return buf;
}

char *jffs3_get_compression_mode_name(void)
{
        switch (jffs3_compression_mode) {
        case JFFS3_COMPR_MODE_NONE:
                return "none";
        case JFFS3_COMPR_MODE_PRIORITY:
                return "priority";
        case JFFS3_COMPR_MODE_SIZE:
                return "size";
        }
        return "unkown";
}

int jffs3_set_compression_mode_name(const char *name)
{
        if (!strcmp("none",name)) {
                jffs3_compression_mode = JFFS3_COMPR_MODE_NONE;
                return 0;
        }
        if (!strcmp("priority",name)) {
                jffs3_compression_mode = JFFS3_COMPR_MODE_PRIORITY;
                return 0;
        }
        if (!strcmp("size",name)) {
                jffs3_compression_mode = JFFS3_COMPR_MODE_SIZE;
                return 0;
        }
        return 1;
}

static int jffs3_compressor_Xable(const char *name, int disabled)
{
        struct jffs3_compressor *this;
        spin_lock(&jffs3_compressor_list_lock);
        list_for_each_entry(this, &jffs3_compressor_list, list) {
                if (!strcmp(this->name, name)) {
                        this->disabled = disabled;
                        spin_unlock(&jffs3_compressor_list_lock);
                        return 0;
                }
        }
        spin_unlock(&jffs3_compressor_list_lock);
        WARNING_MSG("Compressor %s not found.\n", name);
        return 1;
}

int jffs3_enable_compressor_name(const char *name)
{
        return jffs3_compressor_Xable(name, 0);
}

int jffs3_disable_compressor_name(const char *name)
{
        return jffs3_compressor_Xable(name, 1);
}

int jffs3_set_compressor_priority(const char *name, int priority)
{
        struct jffs3_compressor *this,*comp;
        spin_lock(&jffs3_compressor_list_lock);
        list_for_each_entry(this, &jffs3_compressor_list, list) {
                if (!strcmp(this->name, name)) {
                        this->priority = priority;
                        comp = this;
                        goto reinsert;
                }
        }
        spin_unlock(&jffs3_compressor_list_lock);
        WARNING_MSG("Compressor %s not found.\n",name);
        return 1;
reinsert:
        /* list is sorted in the order of priority, so if
           we change it we have to reinsert it into the
           good place */
        list_del(&comp->list);
        list_for_each_entry(this, &jffs3_compressor_list, list) {
                if (this->priority < comp->priority) {
                        list_add(&comp->list, this->list.prev);
                        spin_unlock(&jffs3_compressor_list_lock);
                        return 0;
                }
        }
        list_add_tail(&comp->list, &jffs3_compressor_list);
        spin_unlock(&jffs3_compressor_list_lock);
        return 0;
}

#endif

void jffs3_free_comprbuf(unsigned char *comprbuf, unsigned char *orig)
{
        if (orig != comprbuf)
                kfree(comprbuf);
}

int jffs3_compressors_init(void)
{
/* Registering compressors */
#ifdef CONFIG_JFFS3_ZLIB
        jffs3_zlib_init();
#endif
#ifdef CONFIG_JFFS3_RTIME
        jffs3_rtime_init();
#endif
#ifdef CONFIG_JFFS3_RUBIN
        jffs3_rubinmips_init();
        jffs3_dynrubin_init();
#endif
#ifdef CONFIG_JFFS3_LZARI
        jffs3_lzari_init();
#endif
#ifdef CONFIG_JFFS3_LZO
        jffs3_lzo_init();
#endif
/* Setting default compression mode */
#ifdef CONFIG_JFFS3_CMODE_NONE
        jffs3_compression_mode = JFFS3_COMPR_MODE_NONE;
        DBG_COMPR(1, "Default compression mode: none\n");
#else
#ifdef CONFIG_JFFS3_CMODE_SIZE
        jffs3_compression_mode = JFFS3_COMPR_MODE_SIZE;
        DBG_COMPR(1, "Default compression mode: size\n");
#else
        DBG_COMPR(1, "Default compression mode: priority\n");
#endif
#endif
        return 0;
}

int jffs3_compressors_exit(void)
{
/* Unregistering compressors */
#ifdef CONFIG_JFFS3_LZO
        jffs3_lzo_exit();
#endif
#ifdef CONFIG_JFFS3_LZARI
        jffs3_lzari_exit();
#endif
#ifdef CONFIG_JFFS3_RUBIN
        jffs3_dynrubin_exit();
        jffs3_rubinmips_exit();
#endif
#ifdef CONFIG_JFFS3_RTIME
        jffs3_rtime_exit();
#endif
#ifdef CONFIG_JFFS3_ZLIB
        jffs3_zlib_exit();
#endif
        return 0;
}

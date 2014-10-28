#
# Out of tree fs/jffs3/Makefile
#
# $Id: Makefile,v 3.2 2004/12/17 13:20:30 dedekind Exp $

ifdef OUT_OF_TREE_BUILD
include $(mtd)/defconfig

# This must be first in the include path, so it goes in $(CC) rather
# then $(EXTRA_CFLAGS)

CC += -I$(mtd)/../../include
EXTRA_CFLAGS := -g -Werror

ifndef CONFIG_MTD
EXTRA_CFLAGS += -DMTD_OUT_OF_TREE
endif

ifdef NONAND
EXTRA_CFLAGS += -DNONAND
endif
endif
include $(obj)/Makefile.common

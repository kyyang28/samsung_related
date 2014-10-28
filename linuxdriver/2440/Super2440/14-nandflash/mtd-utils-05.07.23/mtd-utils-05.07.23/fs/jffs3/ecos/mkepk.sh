#!/bin/sh
# JFFS2 Id: mkepk.sh,v 1.8 2004/11/11 20:47:00 lunn  Exp
 * $Id: mkepk.sh,v 3.1 2004/12/09 16:05:53 dwmw2 Exp $

EPK_TMPDIR=/tmp/jffs3-epk-$$
MTDDIR=`dirname $0`/../../..

COREFILES="build.c compr_zlib.c LICENCE scan.c compr.c compr.h gc.c pushpull.h compr_rtime.c histo.h nodelist.c read.c write.c compr_rubin.c erase.c histo_mips.h nodelist.h readinode.c compr_rubin.h nodemgmt.c"
ECOSFILES="os-ecos.h flashio.c gcthread.c dir-ecos.c  fs-ecos.c  malloc-ecos.c"
INCFILES="jffs3.h jffs3_fs_i.h jffs3_fs_sb.h"
DOCFILES="README.Locking TODO ecos/doc/TODO.eCos ecos/doc/readme.txt"
mkdir $EPK_TMPDIR || exit 1

mkdir -p $EPK_TMPDIR/fs/jffs3/current/src
mkdir -p $EPK_TMPDIR/fs/jffs3/current/cdl
mkdir -p $EPK_TMPDIR/fs/jffs3/current/include/linux
mkdir -p $EPK_TMPDIR/fs/jffs3/current/tests
mkdir -p $EPK_TMPDIR/fs/jffs3/current/doc

cat > $EPK_TMPDIR/pkgadd.db <<EOF
package CYGPKG_FS_JFFS3 {
        alias           { "JFFS3 Filesystem" jffs3 }
        directory       fs/jffs3
        script          jffs3.cdl
        description "
           This package contains the JFFS3 filesystem."
}
EOF
for a in $COREFILES; do
	cp -v $MTDDIR/fs/jffs3/$a $EPK_TMPDIR/fs/jffs3/current/src || exit 1
done
for a in $ECOSFILES; do
	cp -v $MTDDIR/fs/jffs3/ecos/src/$a $EPK_TMPDIR/fs/jffs3/current/src || exit 1
done
for a in $INCFILES; do
	cp -v $MTDDIR/include/linux/$a $EPK_TMPDIR/fs/jffs3/current/include/linux || exit 1
done
for a in $DOCFILES; do
	cp -v $MTDDIR/fs/jffs3/$a $EPK_TMPDIR/fs/jffs3/current/doc || exit 1
done
cp -v $MTDDIR/fs/jffs3/ecos/cdl/jffs3.cdl $EPK_TMPDIR/fs/jffs3/current/cdl
cp -v $MTDDIR/fs/jffs3/ecos/tests/*.c $EPK_TMPDIR/fs/jffs3/current/tests
cp -v $MTDDIR/fs/jffs3/ecos/ChangeLog $EPK_TMPDIR/fs/jffs3/current

EPKNAME=jffs3-`date +"%Y%m%d%H%M"`.epk
tar cvfz $EPKNAME -C $EPK_TMPDIR pkgadd.db fs/jffs3

rm -rf $EPK_TMPDIR

echo "Created $PWD/$EPKNAME"

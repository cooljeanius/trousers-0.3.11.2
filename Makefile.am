ACLOCAL_AMFLAGS = -I m4 --install

SUBDIRS = src man dist
EXTRA_DIST = AUTHORS LICENSE NEWS NICETOHAVES README \
	README.selinux TODO ChangeLog \
	doc/LTC-TSS_LLD_08_r2.pdf \
	doc/LTC-TSS_LLD_08_r2.sxw \
	doc/TSS_programming_SNAFUs.txt

CLEANFILES = po/POTFILES

DISTCLEANFILES = *~ autoscan.log build-aux/*~

distclean-local:
	-rm -rf autom4te.cache || rmdir autom4te.cache

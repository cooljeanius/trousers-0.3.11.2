lib_LIBRARIES=libtddl.a

libtddl_a_SOURCES=tddl.c
libtddl_a_CFLAGS=-DAPPID=\"TCSD\ TDDL\" -I${top_srcdir}/src/include -fPIE -DPIE

distclean-local:
	-rm -rf .deps || rmdir .deps || stat .deps

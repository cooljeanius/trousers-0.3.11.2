sbin_PROGRAMS=tcsd

tcsd_CFLAGS=-DAPPID=\"TCSD\" -DVAR_PREFIX=\"@localstatedir@\" -DETC_PREFIX=\"@sysconfdir@\" -I${top_srcdir}/src/include -fPIE -DPIE
tcsd_LDADD=${top_builddir}/src/tcs/libtcs.a ${top_builddir}/src/tddl/libtddl.a -lpthread @CRYPTOLIB@
tcsd_LDFLAGS=-pie
EXTRA_tcsd_UNUSED_LDFLAGS_THAT_ARE_BAD = -Wl,-z,relro -Wl,-z,now

tcsd_SOURCES=svrside.c tcsd_conf.c tcsd_threads.c platform.c

if TSS_BUILD_PS
tcsd_CFLAGS+=-DTSS_BUILD_PS
endif
if TSS_BUILD_PCR_EVENTS
tcsd_CFLAGS+=-DTSS_BUILD_PCR_EVENTS
endif

distclean-local:
	-rm -rf .deps || rmdir .deps || find . -name '.deps' -delete

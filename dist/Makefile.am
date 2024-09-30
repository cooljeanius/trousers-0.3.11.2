EXTRA_DIST = system.data.auth system.data.noauth \
       fedora/fedora.initrd.tcsd
install-data-local: install-exec-hook
	if test ! -e ${DESTDIR}/@sysconfdir@/tcsd.conf; then mkdir -p ${DESTDIR}/@sysconfdir@ && cp tcsd.conf ${DESTDIR}/@sysconfdir@; fi
	/bin/chown tss:tss ${DESTDIR}/@sysconfdir@/tcsd.conf || echo "chowning conffile failed"
	/bin/chmod 0600 ${DESTDIR}/@sysconfdir@/tcsd.conf

install-exec-hook:
	/usr/sbin/groupadd tss || echo "groupadd failed"
	/usr/sbin/useradd -r tss -g tss || echo "useradd failed"
	/bin/sh -c 'if [ ! -e ${DESTDIR}/@localstatedir@/lib/tpm ];then mkdir -p ${DESTDIR}/@localstatedir@/lib/tpm; fi'
	/bin/chown tss:tss ${DESTDIR}/@localstatedir@/lib/tpm || echo "chowning tpm failed"
	/bin/chmod 0700 ${DESTDIR}/@localstatedir@/lib/tpm

uninstall-hook:
	/usr/sbin/userdel tss || echo "userdel failed"
	/usr/sbin/groupdel tss || echo "groupdel failed"
	rm ${DESTDIR}/@sysconfdir@/tcsd.conf
	rmdir ${DESTDIR}/@localstatedir@/lib/tpm

#!/bin/sh


PID=$$
OS=`uname -s | tr '[:lower:]' '[:upper:]'`

CFLAGS="-D_${OS} -Wall" # suppose gcc

if test "X$OS" = "XAIX"; then
    unset CFLAGS

    # xlc or gcc ?
    echo "main(){}" >${PID}.c
    cc -o ${PID}.o -c ${PID}.c -qlist >/dev/null 2>&1
    if head -1 ${PID}.lst 2>/dev/null | grep -i 'XL C for AIX' >/dev/null 2>&1; then
        CFLAGS="-qflag=w:w"
        if ls -l /unix | grep unix_64 >/dev/null 2>&1; then
            CFLAGS="-D_AIX64 -q64 $CFLAGS"
        else
            CFLAGS="-D_AIX $CFLAGS"
        fi
    else
        CFLAGS="-Wall" # suppose gcc
        if ls -l /unix | grep unix_64 >/dev/null 2>&1; then
            CFLAGS="-D_AIX64 -maix64 $CFLAGS"
        else
            CFLAGS="-D_AIX $CFLAGS"
        fi
    fi
    rm -f ${PID}.* >/dev/null 2>&1
fi

#-+- Test in linker to find the pcap library -+-
cat >./${PID}.c <<\EOF
#include <pcap.h>
main() { pcap_lib_version(); }
EOF

LIBS=
LIBPCAP=`find /usr/lib -name "libpcap.a" -print | tail -1`
if test ! -z "$LIBPCAP" ; then
        LIBS="-L`dirname $LIBPCAP` -lpcap"
else
LIBDIR="$LIBDIR /usr/local/lib /usr/lib64 /usr/lib"
for folder in $LIBDIR; do
    if test -d $folder && ls -1 $folder | grep -w libpcap >/dev/null 2>&1; then
        LIBS="-L$folder -lpcap"
        cc -o ${PID}.x ${PID}.c $LIBS >/dev/null 2>&1 && break
    fi
    unset LIBS
done
fi

if test "$LIBS"; then

cat >./${PID}.c <<\EOF  
#include <pcap.h>
#if defined(_AIX) || defined(_AIX64)
#include <net/bpf.h>
#endif

main() {
  struct bpf_program bp;
  pcap_freecode(&bp);
}
EOF
cc -o ${PID}.x -D_$OS ${PID}.c $LIBS >/dev/null 2>&1 && \
CFLAGS="-DHAVE_PCAP_FREECODE $CFLAGS"

cat >./${PID}.c <<\EOF  
#include <pcap.h>
#if defined(_AIX) || defined(_AIX64)
#include <net/bpf.h>
#endif

main() {
  pcap_t *p;
  pcap_breakloop(p);
}
EOF
cc -o ${PID}.x -D_$OS ${PID}.c $LIBS >/dev/null 2>&1 && \
CFLAGS="-DHAVE_PCAP_BREAKLOOP $CFLAGS"

fi

rm -rf ${PID}.* 2>/dev/null

echo "$CFLAGS%$LIBS"


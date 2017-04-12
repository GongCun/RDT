ROOT=..
PLATFORM=$(shell uname -s | tr '[:lower:]' '[:upper:]')

ifneq (,$(findstring xlc,$(shell which $(CC) 2>/dev/null)))
	include $(ROOT)/Make.defines.xlc
else
	include $(ROOT)/Make.defines.gcc
endif
CFLAGS += -D_$(PLATFORM)

LIB_OBJS = error.o wrapunix.o get_ifi_info.o free_ifi_info.o \
	   checksum.o dispatch_pcap.o loop_pcap.o open_pcap.o \
	   signal_func.o ux.o
LIBTCPI_NAME = ../libtcpi.a
RANLIB = ranlib

all:	${LIB_OBJS}
		ar ${AROPT} ${LIBTCPI_NAME} $?
		${RANLIB} ${LIBTCPI_NAME}

clean:
		rm -f ${CLEANFILES} ${LIBTCPI_NAME}

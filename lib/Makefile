ROOT=..
ifneq (,$(findstring xlc,$(shell which $(CC) 2>/dev/null)))
	include $(ROOT)/Make.defines.xlc
else
	include $(ROOT)/Make.defines
endif

LIB_OBJS = error.o wrapunix.o get_ifi_info.o free_ifi_info.o \
	   checksum.o dispatch_pcap.o loop_pcap.o open_pcap.o \
	   signal_func.o ux.o

all:	${LIB_OBJS}

$(LIBTCPI): $(LIB_OBJS)
	$(AR) rv $(LIBTCPI) $?
	${RANLIB} ${LIBTCPI}

clean:
	rm -f ${CLEANFILES} ${LIBTCPI}

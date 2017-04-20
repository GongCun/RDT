ROOT=.

ifneq (,$(findstring xlc,$(shell which $(CC) 2>/dev/null)))
	include $(ROOT)/Make.defines.xlc
else
	include $(ROOT)/Make.defines
endif

LIBRDT = librdt.a
PROGS = t_send t_recv rdt
OBJS  = get_addr.o get_dev.o get_mtu.o make_pkt.o to_net.o \
	from_net.o krdt_connect.o make_sock.o oper_fifo.o \
	chk_chksum.o pkt_arrive.o rtt.o rexmt_pkt.o \
	pkt_debug.o krdt_listen.o rdt_connect.o rdt_listen.o \
	conn_info_debug.o conn_user_debug.o pass_pkt.o get_pkt.o \
	rdt_send.o rdt_recv.o rdt_close.o conn_debug.o rdt_pipe.o \
	conn_alloc.o rdt_fin.o

all: ${PROGS} ${LIBRDT}

rdt: 	rdt.o ${LIBRDT} $(LIBTCPI)
	${CC} ${CFLAGS} -o $@ $< $(LIBRDT) $(LIBS)

t_send:	t_send.o ${LIBRDT} $(LIBTCPI)
	${CC} ${CFLAGS} -o $@ $< $(LIBRDT) $(LIBS)

t_recv: t_recv.o ${LIBRDT} $(LIBTCPI)
	${CC} ${CFLAGS} -o $@ $< $(LIBRDT) $(LIBS)

${OBJS}: rdt.h

${LIBTCPI}:
	cd $(ROOT)/lib && $(MAKE)

${LIBRDT}: ${OBJS}
	$(AR) rv $(LIBRDT) $?
	$(RANLIB) $(LIBRDT)

clean:
	cd $(ROOT)/lib && $(MAKE) clean
	rm -rf ${PROGS} ${CLEANFILES}

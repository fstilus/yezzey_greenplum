#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for libpq subsystem (backend half of libpq interface)
#
# IDENTIFICATION
#    src/backend/libpq/Makefile
#
#-------------------------------------------------------------------------

subdir = src/backend/libpq
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global
override CPPFLAGS += -I$(libpq_srcdir) -I$(top_srcdir)/src/port

# be-fsstubs is here for historical reasons, probably belongs elsewhere

OBJS = fe-auth-scram.o be-fsstubs.o be-secure.o auth.o crypt.o hba.o ip.o md5.o pqcomm.o \
       pqformat.o pqsignal.o auth-scram.o pg_sha2.o fe-protocol3.o fe-connect.o \
       fe-exec.o pqexpbuffer.o fe-auth.o fe-misc.o fe-protocol2.o fe-secure.o \
       $(filter getpeereid.o, $(LIBOBJS))

ifeq ($(with_gssapi),yes)
OBJS += be-gssapi-common.o
endif

fe-protocol3.c fe-auth-scram.c fe-connect.c fe-exec.c pqexpbuffer.c fe-auth.c fe-misc.c fe-protocol2.c fe-secure.c: % : $(top_srcdir)/src/interfaces/libpq/%
	rm -f $@ && $(LN_S) $< .

getpeereid.c: % : $(top_srcdir)/src/port/%
	rm -f $@ && $(LN_S) $< .

fe-connect.o: fe-connect.c $(top_builddir)/src/port/pg_config_paths.h

$(top_builddir)/src/port/pg_config_paths.h:
	$(MAKE) -C $(top_builddir)/src/port pg_config_paths.h

clean distclean: clean-symlinks

clean-symlinks:
	rm -f fe-protocol3.c fe-auth-scram.c fe-connect.c fe-exec.c pqexpbuffer.c fe-auth.c fe-misc.c fe-protocol2.c fe-secure.c getpeereid.c

include $(top_srcdir)/src/backend/common.mk

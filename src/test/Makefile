#-------------------------------------------------------------------------
#
# Makefile for src/test
#
# Copyright (c) 1994, Regents of the University of California
#
# src/test/Makefile
#
#-------------------------------------------------------------------------

subdir = src/test
top_builddir = ../..
include $(top_builddir)/src/Makefile.global

SUBDIRS = perl regress isolation authentication recovery

SUBDIRS += fsync walrep heap_checksum isolation2 ssl kerberos modules fdw

$(recurse)

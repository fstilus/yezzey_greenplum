# contrib/btree_gin/Makefile

MODULE_big = btree_gin
OBJS = btree_gin.o

EXTENSION = btree_gin
DATA = btree_gin--1.0.sql btree_gin--unpackaged--1.0.sql

REGRESS = install_btree_gin int2 int4 int8 float4 float8 money oid \
	timestamp timestamptz time timetz date interval \
	macaddr inet cidr text varchar char bytea bit varbit \
	numeric
REGRESS_OPTS += --init-file=$(top_builddir)/src/test/regress/init_file

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/btree_gin
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

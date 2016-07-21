AUTOMAKE_OPTIONS=foreign
SUBDIRS=common config database transaction
bin_PROGRAMS=gMaxLinked
gMaxLinked_SOURCES=gMaxLinked.cpp
gMaxLinked_LDFLAGS=-lpthread -llog4c -lhiredis
gMaxLinked_LDADD=transaction/libtransaction.a database/libdatabase.a config/libconfig.a common/libcommon.a 

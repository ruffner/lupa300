#############################################################################
# Makefile for building: libQtZeroConf.so.1.0.0
# Generated by qmake (2.01a) (Qt 4.8.6) on: Mon Apr 10 19:32:51 2017
# Project:  qtzeroconf.pro
# Template: lib
# Command: /usr/local/Trolltech/Qt-4.8.6/bin/qmake -o Makefile qtzeroconf.pro
#############################################################################

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -DQT_BUILD_ZEROCONF_LIB -DQT_NO_DEBUG -DQT_NETWORK_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT -fPIC $(DEFINES)
CXXFLAGS      = -pipe -I/root/Engineering/tcpip/QtZeroConf-master -O2 -Wall -W -D_REENTRANT -fPIC $(DEFINES)
INCPATH       = -I/usr/local/Trolltech/Qt-4.8.6/mkspecs/linux-g++ -I. -I/usr/local/Trolltech/Qt-4.8.6/include/QtCore -I/usr/local/Trolltech/Qt-4.8.6/include/QtNetwork -I/usr/local/Trolltech/Qt-4.8.6/include -I.
LINK          = g++
LFLAGS        = -Wl,-O1 -Wl,-rpath,/usr/local/Trolltech/Qt-4.8.6/lib -shared -Wl,-soname,libQtZeroConf.so.1
LIBS          = $(SUBLIBS)  -L/usr/local/Trolltech/Qt-4.8.6/lib -lavahi-client -lavahi-common -lQtNetwork -L/usr/local/Trolltech/Qt-4.8.6/lib -lQtCore -lpthread 
AR            = ar cqs
RANLIB        = 
QMAKE         = /usr/local/Trolltech/Qt-4.8.6/bin/qmake
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = avahiclient.cpp \
		avahi-qt/qt-watch.cpp moc_qzeroconf.cpp \
		moc_qt-watch_p.cpp
OBJECTS       = avahiclient.o \
		qt-watch.o \
		moc_qzeroconf.o \
		moc_qt-watch_p.o
DIST          = /usr/local/Trolltech/Qt-4.8.6/mkspecs/common/unix.conf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/linux.conf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/gcc-base.conf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/gcc-base-unix.conf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/g++-base.conf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/g++-unix.conf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/qconfig.pri \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/qt_functions.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/qt_config.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/exclusive_builds.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/default_pre.prf \
		qtzeroconf.pri \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/release.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/default_post.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/shared.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/dll.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/unix/gdb_dwarf_index.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/warn_on.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/qt.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/unix/thread.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/moc.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/resources.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/uic.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/yacc.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/lex.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/include_source_dir.prf \
		qtzeroconf.pro
QMAKE_TARGET  = QtZeroConf
DESTDIR       = 
TARGET        = libQtZeroConf.so.1.0.0
TARGETA       = libQtZeroConf.a
TARGETD       = libQtZeroConf.so.1.0.0
TARGET0       = libQtZeroConf.so
TARGET1       = libQtZeroConf.so.1
TARGET2       = libQtZeroConf.so.1.0

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile libQtZeroConf.prl  $(TARGET)

$(TARGET):  $(OBJECTS) $(SUBLIBS) $(OBJCOMP)  
	-$(DEL_FILE) $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) $(OBJCOMP)
	-ln -s $(TARGET) $(TARGET0)
	-ln -s $(TARGET) $(TARGET1)
	-ln -s $(TARGET) $(TARGET2)



staticlib: $(TARGETA)

$(TARGETA):  $(OBJECTS) $(OBJCOMP) 
	-$(DEL_FILE) $(TARGETA) 
	$(AR) $(TARGETA) $(OBJECTS)

libQtZeroConf.prl: 
	@$(QMAKE) -prl  qtzeroconf.pro
Makefile: qtzeroconf.pro  /usr/local/Trolltech/Qt-4.8.6/mkspecs/linux-g++/qmake.conf /usr/local/Trolltech/Qt-4.8.6/mkspecs/common/unix.conf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/linux.conf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/gcc-base.conf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/gcc-base-unix.conf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/g++-base.conf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/g++-unix.conf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/qconfig.pri \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/qt_functions.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/qt_config.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/exclusive_builds.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/default_pre.prf \
		qtzeroconf.pri \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/release.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/default_post.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/shared.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/dll.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/unix/gdb_dwarf_index.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/warn_on.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/qt.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/unix/thread.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/moc.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/resources.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/uic.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/yacc.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/lex.prf \
		/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/include_source_dir.prf \
		/usr/local/Trolltech/Qt-4.8.6/lib/libQtNetwork.prl \
		/usr/local/Trolltech/Qt-4.8.6/lib/libQtCore.prl
	$(QMAKE) -o Makefile qtzeroconf.pro
/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/unix.conf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/linux.conf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/gcc-base.conf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/gcc-base-unix.conf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/g++-base.conf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/common/g++-unix.conf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/qconfig.pri:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/qt_functions.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/qt_config.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/exclusive_builds.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/default_pre.prf:
qtzeroconf.pri:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/release.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/default_post.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/shared.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/dll.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/unix/gdb_dwarf_index.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/warn_on.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/qt.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/unix/thread.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/moc.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/resources.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/uic.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/yacc.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/lex.prf:
/usr/local/Trolltech/Qt-4.8.6/mkspecs/features/include_source_dir.prf:
/usr/local/Trolltech/Qt-4.8.6/lib/libQtNetwork.prl:
/usr/local/Trolltech/Qt-4.8.6/lib/libQtCore.prl:
qmake:  FORCE
	@$(QMAKE) -o Makefile qtzeroconf.pro

dist: 
	@$(CHK_DIR_EXISTS) .tmp/QtZeroConf1.0.0 || $(MKDIR) .tmp/QtZeroConf1.0.0 
	$(COPY_FILE) --parents $(SOURCES) $(DIST) .tmp/QtZeroConf1.0.0/ && $(COPY_FILE) --parents qzeroconf.h avahi-qt/qt-watch.h avahi-qt/qt-watch_p.h .tmp/QtZeroConf1.0.0/ && $(COPY_FILE) --parents avahiclient.cpp avahi-qt/qt-watch.cpp .tmp/QtZeroConf1.0.0/ && (cd `dirname .tmp/QtZeroConf1.0.0` && $(TAR) QtZeroConf1.0.0.tar QtZeroConf1.0.0 && $(COMPRESS) QtZeroConf1.0.0.tar) && $(MOVE) `dirname .tmp/QtZeroConf1.0.0`/QtZeroConf1.0.0.tar.gz . && $(DEL_FILE) -r .tmp/QtZeroConf1.0.0


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) $(TARGET0) $(TARGET1) $(TARGET2) $(TARGETA)
	-$(DEL_FILE) Makefile


check: first

mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

compiler_moc_header_make_all: moc_qzeroconf.cpp moc_qt-watch_p.cpp
compiler_moc_header_clean:
	-$(DEL_FILE) moc_qzeroconf.cpp moc_qt-watch_p.cpp
moc_qzeroconf.cpp: qzeroconf.h
	/usr/local/Trolltech/Qt-4.8.6/bin/moc $(DEFINES) $(INCPATH) qzeroconf.h -o moc_qzeroconf.cpp

moc_qt-watch_p.cpp: avahi-common/watch.h \
		avahi-common/cdecl.h \
		avahi-qt/qt-watch_p.h
	/usr/local/Trolltech/Qt-4.8.6/bin/moc $(DEFINES) $(INCPATH) avahi-qt/qt-watch_p.h -o moc_qt-watch_p.cpp

compiler_rcc_make_all:
compiler_rcc_clean:
compiler_image_collection_make_all: qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) qmake_image_collection.cpp
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_uic_make_all:
compiler_uic_clean:
compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: compiler_moc_header_clean 

####### Compile

avahiclient.o: avahiclient.cpp avahi-qt/qt-watch.h \
		avahi-common/watch.h \
		avahi-common/cdecl.h \
		avahi-common/error.h \
		qzeroconf.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o avahiclient.o avahiclient.cpp

qt-watch.o: avahi-qt/qt-watch.cpp avahi-common/timeval.h \
		avahi-common/cdecl.h \
		avahi-qt/qt-watch.h \
		avahi-common/watch.h \
		avahi-qt/qt-watch_p.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o qt-watch.o avahi-qt/qt-watch.cpp

moc_qzeroconf.o: moc_qzeroconf.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_qzeroconf.o moc_qzeroconf.cpp

moc_qt-watch_p.o: moc_qt-watch_p.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_qt-watch_p.o moc_qt-watch_p.cpp

####### Install

install_target: first FORCE
	@$(CHK_DIR_EXISTS) $(INSTALL_ROOT)/lib/ || $(MKDIR) $(INSTALL_ROOT)/lib/ 
	-$(INSTALL_PROGRAM) "$(TARGET)" "$(INSTALL_ROOT)/lib/$(TARGET)"
	-$(STRIP) --strip-unneeded "$(INSTALL_ROOT)/lib/$(TARGET)"
	-$(SYMLINK) "$(TARGET)" "$(INSTALL_ROOT)/lib/$(TARGET0)"
	-$(SYMLINK) "$(TARGET)" "$(INSTALL_ROOT)/lib/$(TARGET1)"
	-$(SYMLINK) "$(TARGET)" "$(INSTALL_ROOT)/lib/$(TARGET2)"
	@$(CHK_DIR_EXISTS) $(INSTALL_ROOT)/lib || $(MKDIR) $(INSTALL_ROOT)/lib 
	-$(INSTALL_FILE) "libQtZeroConf.prl" "$(INSTALL_ROOT)/lib/libQtZeroConf.prl"

uninstall_target:  FORCE
	-$(DEL_FILE) "$(INSTALL_ROOT)/lib/$(TARGET)" 
	 -$(DEL_FILE) "$(INSTALL_ROOT)/lib/$(TARGET0)" 
	 -$(DEL_FILE) "$(INSTALL_ROOT)/lib/$(TARGET1)" 
	 -$(DEL_FILE) "$(INSTALL_ROOT)/lib/$(TARGET2)" 
	 -$(DEL_FILE) "$(INSTALL_ROOT)/lib/libQtZeroConf.prl"
	-$(DEL_DIR) $(INSTALL_ROOT)/lib/ 


install:  install_target  FORCE

uninstall: uninstall_target   FORCE

FORCE:


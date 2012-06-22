INCPATH = -I..
VIZMO_DIR = ../..
include $(VIZMO_DIR)/M_incl

ifeq ($(RPM), 0)
TARGET   = ../../lib/libvgui.a
else
TARGET   = ../../lib/libvgui.so
endif

####### Output directory

OBJECTS = GUI.o	MainWin.o scene_win.o AnimationGUI.o \
          snapshot_gui.o ItemSelectionGUI.o FileListDialog.o \
	  ObjProperty.o roadmap.o OBPRMGUI.o AddObjDialog.o\
	  vizmoEditor.o queryGUI.o

SRCMOC = moc_MainWin.cpp moc_scene_win.cpp moc_AnimationGUI.cpp moc_snapshot_gui.cpp \
         moc_ItemSelectionGUI.cpp moc_roadmap.cpp moc_FileListDialog.cpp moc_ObjProperty.cpp \
	 moc_OBPRMGUI.cpp moc_AddObjDialog.cpp moc_vizmoEditor.cpp moc_AttributeSelectionGUI.cpp moc_queryGUI.cpp
OBJMOC = moc_MainWin.o moc_scene_win.o moc_AnimationGUI.o moc_snapshot_gui.o moc_ItemSelectionGUI.o \
         moc_roadmap.o moc_FileListDialog.o moc_ObjProperty.o moc_OBPRMGUI.o \
	 moc_AddObjDialog.o moc_vizmoEditor.o moc_queryGUI.o


first: all

####### Build rules

ifeq ($(RPM), 0)
$(TARGET): $(OBJECTS) $(OBJMOC)
	ar ruv $(TARGET) ${OBJECTS} $(OBJMOC)
#	libtool -static -o $(TARGET) ${OBJECTS} $(OBJMOC)
else
$(TARGET): $(OBJECTS) $(OBJMOC)
	$(CC) -shared $(OBJECTS) $(OBJMOC) -Wl,-soname=`basename $(TARGET)`.0 -o $(TARGET)
endif

#$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) $(all) 
	#$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)

mocables: $(SRCMOC)

#$(MOC): 
#	( cd $(QTDIR)/src/moc ; $(MAKE) )

default_target: $(TARGET)

CLEAN = $(OBJMOC) $(SRCMOC) $(OBJECTS) Dependencies *~ core *.core $(TARGET)

FORCE:

####### MOC

moc_MainWin.cpp: $(MOC) MainWin.h
	$(MOC) MainWin.h -o moc_MainWin.cpp

moc_scene_win.cpp: $(MOC) scene_win.h
	$(MOC) scene_win.h -o moc_scene_win.cpp

moc_AnimationGUI.cpp:  $(MOC) AnimationGUI.h
	$(MOC) AnimationGUI.h -o moc_AnimationGUI.cpp

moc_ItemSelectionGUI.cpp: $(MOC) ItemSelectionGUI.h
	$(MOC) ItemSelectionGUI.h -o moc_ItemSelectionGUI.cpp

moc_roadmap.cpp: $(MOC) roadmap.h
	$(MOC) roadmap.h -o moc_roadmap.cpp

moc_snapshot_gui.cpp:  $(MOC) snapshot_gui.h
	$(MOC) snapshot_gui.h -o moc_snapshot_gui.cpp

moc_FileListDialog.cpp: $(MOC) FileListDialog.h
	$(MOC) FileListDialog.h -o moc_FileListDialog.cpp

moc_ObjProperty.cpp: $(MOC) ObjProperty.h
	$(MOC) ObjProperty.h -o moc_ObjProperty.cpp

moc_OBPRMGUI.cpp: $(MOC) OBPRMGUI.h 
	$(MOC) OBPRMGUI.h -o moc_OBPRMGUI.cpp

moc_AddObjDialog.cpp: $(MOC) AddObjDialog.h
	$(MOC) AddObjDialog.h -o moc_AddObjDialog.cpp

moc_vizmoEditor.cpp: $(MOC) vizmoEditor.h
	$(MOC) vizmoEditor.h -o moc_vizmoEditor.cpp

moc_queryGUI.cpp: $(MOC) queryGUI.h
	$(MOC) queryGUI.h -o moc_queryGUI.cpp

-include Dependencies

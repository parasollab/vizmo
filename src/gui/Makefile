INCPATH = -I..
VIZMO_DIR = ../..
include $(VIZMO_DIR)/M_incl

ifeq ($(RPM), 0)
TARGET   = ../../lib/libvgui.a
else
TARGET   = ../../lib/libvgui.so
endif

####### Output directory

OBJECTS = GUI.o	main_win.o scene_win.o AnimationGUI.o \
          snapshot_gui.o itemselection_gui.o FileListDialog.o \
	  obj_property.o roadmap.o obprmGUI.o AddObjDialog.o\
	  vizmoEditor.o queryGUI.o

SRCMOC = moc_main_win.cpp moc_scene_win.cpp moc_AnimationGUI.cpp moc_snapshot_gui.cpp \
         moc_itemselection_gui.cpp moc_roadmap.cpp moc_FileListDialog.cpp moc_obj_property.cpp \
	 moc_obprmGUI.cpp moc_AddObjDialog.cpp moc_vizmoEditor.cpp moc_AttributeSelectionGUI.cpp moc_queryGUI.cpp
OBJMOC = moc_main_win.o moc_scene_win.o moc_AnimationGUI.o moc_snapshot_gui.o moc_itemselection_gui.o \
         moc_roadmap.o moc_FileListDialog.o moc_obj_property.o moc_obprmGUI.o \
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

moc_main_win.cpp: $(MOC) main_win.h
	$(MOC) main_win.h -o moc_main_win.cpp

moc_scene_win.cpp: $(MOC) scene_win.h
	$(MOC) scene_win.h -o moc_scene_win.cpp

moc_AnimationGUI.cpp:  $(MOC) AnimationGUI.h
	$(MOC) AnimationGUI.h -o moc_AnimationGUI.cpp

moc_itemselection_gui.cpp: $(MOC) itemselection_gui.h
	$(MOC) itemselection_gui.h -o moc_itemselection_gui.cpp

moc_roadmap.cpp: $(MOC) roadmap.h
	$(MOC) roadmap.h -o moc_roadmap.cpp

moc_snapshot_gui.cpp:  $(MOC) snapshot_gui.h
	$(MOC) snapshot_gui.h -o moc_snapshot_gui.cpp

moc_FileListDialog.cpp: $(MOC) FileListDialog.h
	$(MOC) FileListDialog.h -o moc_FileListDialog.cpp

moc_obj_property.cpp: $(MOC) obj_property.h
	$(MOC) obj_property.h -o moc_obj_property.cpp

moc_obprmGUI.cpp: $(MOC) obprmGUI.h 
	$(MOC) obprmGUI.h -o moc_obprmGUI.cpp

moc_AddObjDialog.cpp: $(MOC) AddObjDialog.h
	$(MOC) AddObjDialog.h -o moc_AddObjDialog.cpp

moc_vizmoEditor.cpp: $(MOC) vizmoEditor.h
	$(MOC) vizmoEditor.h -o moc_vizmoEditor.cpp

moc_queryGUI.cpp: $(MOC) queryGUI.h
	$(MOC) queryGUI.h -o moc_queryGUI.cpp

-include Dependencies

#*****************************************************************************
#                                                                            *
# Make file for VMS                                                          *
# Author : J.Jansen (joukj@hrem.nano.tudelft.nl)                             *
# Date : 13 February 2006                                                    *
#                                                                            *
#*****************************************************************************
.first
	define wx [--.include.wx]

.ifdef __WXMOTIF__
CXX_DEFINE = /define=(__WXMOTIF__=1)/name=(as_is,short)\
	   /assume=(nostdnew,noglobal_array_new)
.else
.ifdef __WXGTK__
CXX_DEFINE = /define=(__WXGTK__=1)/float=ieee/name=(as_is,short)/ieee=denorm\
	   /assume=(nostdnew,noglobal_array_new)
.else
.ifdef __WXGTK2__
CXX_DEFINE = /define=(__WXGTK__=1,VMS_GTK2=1)/float=ieee/name=(as_is,short)/ieee=denorm\
	   /assume=(nostdnew,noglobal_array_new)
.else
.ifdef __WXX11__
CXX_DEFINE = /define=(__WXX11__=1,__WXUNIVERSAL__==1)/float=ieee\
	/name=(as_is,short)/assume=(nostdnew,noglobal_array_new)
.else
CXX_DEFINE =
.endif
.endif
.endif
.endif

.suffixes : .cpp

.cpp.obj :
	cxx $(CXXFLAGS)$(CXX_DEFINE) $(MMS$TARGET_NAME).cpp

all :
.ifdef __WXMOTIF__
	$(MMS)$(MMSQUALIFIERS) swpctool.exe
.else
.ifdef __WXGTK__
	$(MMS)$(MMSQUALIFIERS) swpctool_gtk.exe
.else
.ifdef __WXGTK2__
	$(MMS)$(MMSQUALIFIERS) swpctool_gtk2.exe
.else
.ifdef __WXX11__
	$(MMS)$(MMSQUALIFIERS) swpctool_x11.exe
.endif
.endif
.endif
.endif

.ifdef __WXMOTIF__
swpctool.exe : swpctool.obj
	cxxlink swpctool,[--.lib]vms/opt
.else
.ifdef __WXGTK__
swpctool_gtk.exe : swpctool.obj
	cxxlink/exec=swpctool_gtk.exe swpctool,[--.lib]vms_gtk/opt
.else
.ifdef __WXGTK2__
swpctool_gtk2.exe : swpctool.obj
	cxxlink/exec=swpctool_gtk2.exe swpctool,[--.lib]vms_gtk2/opt
.else
.ifdef __WXX11__
swpctool_x11.exe : swpctool.obj
	cxxlink/exec=swpctool_x11.exe swpctool,[--.lib]vms_x11_univ/opt
.endif
.endif
.endif
.endif

swpctool.obj : swpctool.cpp

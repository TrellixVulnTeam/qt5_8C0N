!isEmpty(QMAKE_INCDIR_EGL): INCLUDEPATH += $$QMAKE_INCDIR_EGL
!isEmpty(QMAKE_INCDIR_OPENGL): INCLUDEPATH += $$QMAKE_INCDIR_OPENGL

CONFIG-=qt

unix:SOURCES += khr.cpp

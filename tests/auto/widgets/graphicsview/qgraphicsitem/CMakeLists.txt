# Generated from qgraphicsitem.pro.

#####################################################################
## tst_qgraphicsitem Test:
#####################################################################

add_qt_test(tst_qgraphicsitem
    SOURCES
        tst_qgraphicsitem.cpp
    DEFINES
        QT_NO_CAST_TO_ASCII
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::TestPrivate
        Qt::Widgets
        Qt::WidgetsPrivate
)

## Scopes:
#####################################################################

extend_target(tst_qgraphicsitem CONDITION WIN32
    PUBLIC_LIBRARIES
        user32
)

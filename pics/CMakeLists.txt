kde4_header()

add_subdirectory( custom ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kolourpaint/icons FILES  )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = custom
#
#KDE_ICON = kolourpaint
#
#actionicondir = $(kde_datadir)/kolourpaint/icons
#actionicon_ICON = AUTO
#
## Change the following line every time you add an icon
## to force Makefile regeneration:
##
## 4
##
#

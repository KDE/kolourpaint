
add_subdirectory( custom ) 




########### install files ###############

install( FILES   DESTINATION  ${DATA_INSTALL_DIR}/kolourpaint/icons )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )




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

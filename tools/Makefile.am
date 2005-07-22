INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../cursors -I$(srcdir)/../interfaces \
    -I$(srcdir)/../pixmapfx \
    -I$(srcdir)/../tools \
    -I$(srcdir)/../views \
    -I$(srcdir)/../widgets $(all_includes)

noinst_LTLIBRARIES = libkolourpainttools.la
libkolourpainttools_la_SOURCES = kptoolaction.cpp \
    kptoolairspray.cpp \
    kptoolautocrop.cpp \
    kptoolbrush.cpp kptoolclear.cpp \
    kptoolcolorpicker.cpp kptoolcolorwasher.cpp \
    kptoolconverttograyscale.cpp \
    kptoolcrop.cpp \
    kptoolcurve.cpp \
    kptoolellipse.cpp \
    kptoolellipticalselection.cpp kptooleraser.cpp \
    kptoolflip.cpp kptoolfloodfill.cpp \
    kptoolfreeformselection.cpp \
    kptoolline.cpp kptoolpen.cpp \
    kptoolpolygon.cpp kptoolpolyline.cpp \
    kptoolpreviewdialog.cpp \
    kptoolrectangle.cpp kptoolrectselection.cpp \
    kptoolresizescale.cpp kptoolrotate.cpp \
    kptoolroundedrectangle.cpp kptoolselection.cpp \
    kptoolskew.cpp kptooltext.cpp

# TODO: Why is this needed?  Isn't linking at the toplevel enough?
libkolourpainttools_la_LIBADD = ../pixmapfx/libkolourpaintpixmapfx.la ../cursors/libkolourpaintcursors.la

METASOURCES = kptoolaction.moc \
    kptoolairspray.moc \
    kptoolbrush.moc \
    kptoolcolorpicker.moc \
    kptoolcolorwasher.moc \
    kptoolcurve.moc \
    kptoolellipse.moc \
    kptooleraser.moc \
    kptoolflip.moc \
    kptoolfloodfill.moc \
    kptoolline.moc \
    kptoolpen.moc \
    kptoolpolygon.moc \
    kptoolpolyline.moc \
    kptoolpreviewdialog.moc \
    kptoolrectangle.moc \
    kptoolresizescale.moc \
    kptoolrotate.moc \
    kptoolroundedrectangle.moc \
    kptoolselection.moc \
    kptoolskew.moc \
    kptooltext.moc


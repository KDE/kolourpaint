# TODO: Can we not store:
#           Thumbnails/thumbnail.png
#           layout-cache
#       ?

#
# Build System for
# OpenOffice.org Documents
# Stored in Revision Control
#
# Copyright (c) 2006 Clarence Dang <clarencedang@users.sourceforge.net>
#
#
# We store the unpacked contents of OpenOffice.org ("OOo") documents
# in a folder, instead of the binary .odt/.odp (which is actually a ZIP).
# We regenerate this binary file.
# 
# Advantages:
#
# 1. Meaningful diff's between revisions
#
# 2. Reduces repository size substantially since the revision control
#    system does not have to store xdelta's between binary versions (which
#    change widely due to ZIP compression).
#
#
# To modify an existing document using this scheme:
#
# 1. Change your OOO settings so that the XML is not stored on one line
#    and undiffable.
#    
#    In OOo 2.0, use:
#        Tools / Options ... /
#            Load/Save / General / [not] Size optimization for XML format
#
# 2. After checkout, type "make" to generate the .odt from the folder
#    contents
#
# 3. After changing the .odt, type "make" to regenerate the folder contents.
#    "svn add" any new files and then "svn commit".
#
# This Makefile magically does this.
#
#
# To create documents:
#
# 1. Change your OOO settings so that the XML is not stored on one line
#    and undiffable.
#    
#    In OOo 2.0, use:
#        Tools / Options ... /
#            Load/Save / General / [not] Size optimization for XML format
#
# 2. Create the OpenOffice.org document to be stored in revision control.
#
# 3. Modify the following variables:

# This is for a file called "kolourpaint-developer-guide.odt".
DOC=kolourpaint-developer-guide
EXT=odt

# 4. Unpack "kolourpaint-developer-guide.odt" into kolourpaint-developer-guide/.
#    We are going to keep kolourpaint-developer-guide/ in revision control only:
#
#        mkdir kolourpaint-developer-guide/
#        cd kolourpaint-developer-guide/
#        unzip ../kolourpaint-developer-guide.odt
#        cd ..
#
# 5. Add to revision control
#
#        svn propedit svn:ignore .  [and type kolourpaint-developer-guide.odt]
#        svn add kolourpaint-developer-guide/
#        svn commit
#


#
# No need to modify anything below here.
#

FILE=$(DOC).$(EXT)
DIR=$(DOC)

all : $(FILE) unpack

# Do after checkout to create $(FILE).
$(FILE) :
	# 'make' will only execute this entire rule if $(FILE) does not already exist
	rm -f $(FILE)
	(cd $(DIR) && zip ../$(FILE) `find | fgrep -v .svn`)

# Do before committing.
unpack :
	(cd $(DIR) && yes | unzip ../$(FILE))

clean :
	rm -f $(FILE)

# (force unpack every time)
.PHONY : unpack clean

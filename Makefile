# Simply type 'make' all the time and it will work itself out!
all : talk.odp unpack

# Do after checkout to create talk.odp.
talk.odp :
	# 'make' will only execute this entire rule if talk.odp does not already exist
	rm -f talk.odp
	(cd talk && zip ../talk.odp `find | fgrep -v .svn`)

# Do before committing.
unpack :
	(cd talk && yes | unzip ../talk.odp)

clean :
	rm -f talk.odp

# (force unpack every time)
.PHONY : unpack clean

#!/usr/bin/perl

#
# Converts all "#include <kptheclass.h>" in STDIN to "#include <kpTheClass.h>".
#
# Usage:
#
#     (for f in `find -name \*.h -o -name \*.cpp`; do echo $f; /home/kde4/celerysvn/kolourpaint-control/kde4port/include-camelcase-headers.pl < $f > k; cp k $f; echo;echo;done)
#

use strict;
use warnings;


while (my $line = <STDIN>) {
	if ($line =~ /^#include <(kp[^>]+\.h)>$/) {
		my $header = $1;


		my $newHeader=`for f in \$(find -iname $header); do basename \$f; done`;
		chomp $newHeader;
		if ($newHeader eq "") {
			print STDERR "SKIPPING: Could not find CamelCased version of $header\n";
			print $line;
			next;
		}

		my $headerLower = $header;
		$headerLower =~ tr/A-Z/a-z/;

		my $newHeaderLower = $newHeader;
		$newHeaderLower =~ tr/A-Z/a-z/;

		# In case "find" returned multiple matches and for other unexpected cases.
		if ($newHeaderLower ne $headerLower) {
			print STDERR "SKIPPING: New header '$newHeader' doesn't seem to be equivalent to the old one '$header'\n";
			print $line;
			next;
		}

		print "#include <$newHeader>\n";
	} else {
		print $line;
	}
}

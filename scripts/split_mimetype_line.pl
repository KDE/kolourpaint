#!/usr/bin/perl

# Prints each mime type that's:
#
# 1. on a MimeType= line in a kolourpaint.desktop (semicolon separated)
# OR
# 2. on a comma separated line
#
# on a separate line.

use strict;
use warnings;


while (<>)
{
    my $line = $_;
    
    if ($line =~ s/^MimeType=//)
    {
         print "$_\n" foreach (split /;/, $line);
    }
    else
    {
         print "$_\n" foreach (split /,/, $line);
    }
}

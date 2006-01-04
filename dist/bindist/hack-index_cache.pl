#!/usr/bin/perl

# Replace protocol references in index.cache, with local refs, checking if
# those local refs exists.

use strict;
use warnings;


if (@ARGV != 1)
{
    print "Syntax: $0 <index.cache>\n";
    
    exit 1;
}


my ($file) = @ARGV;


# TODO: surely a better way
#sub copyFile
#{
#    my ($src, $dest) = @_;
#    open (HANDLE, "cp \"$src\" \"$dest\" |") or die;
#    close (HANDLE) or die;  # detect errors
#    
#    return 1;  # OK
#}

sub writeFile
{
    my ($file, $contents) = @_;
    
    open (FILE, ">$file") or die;
    print FILE $contents or die;
    close (FILE) or die;
}

sub doDocbook
{
    my ($file) = @_;
    print "doDocbook ($file)\n";
    
    my %globalFiles;
    
    my $output = "";
    my $lineNo = 1;
    open (FILE, "<$file") or die;
    while (my $line = <FILE>)
    {
        while ($line =~ s/help:\/(([^)\"\n]+\/|)([^)\"\/\n]+))/$3/)
        {
            die if not defined $1;
            die if not defined $2;
            die if not defined $3;
            my ($helpFilePath, $helpFileDir, $targetFile) = ($1, $2, $3);
            
            print "\t$lineNo:$helpFilePath";
            if (not defined $globalFiles {$targetFile})
            {
                die if (not -r $targetFile);
                print ": $targetFile";
                $globalFiles {$targetFile} = 1;
            }
            print" \n";
        }
        $output .= $line;
        
        $lineNo++;
    }
    close (FILE) or die;
    
    writeFile ($file, $output);
    return %globalFiles;
}

doDocbook ($file);

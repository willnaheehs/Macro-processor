#!/usr/bin/perl
#
#    t49.pl [length]
#
#  Linear time/space: Very long VALUE field

$length = (defined $ARGV[0]) ? $ARGV[0] : 1;

$longInput = "a" x $length;
print "Very long VALUE field: \\def{A}{$longInput}\n";

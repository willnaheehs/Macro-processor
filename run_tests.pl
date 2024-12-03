#!/usr/bin/perl -w
use strict;

our $debug = 0;
our $CLASS  = "323";
our $PROJ   = "1";
our $NAME   = "proj1";           # Name of program
our $LANG   = "C";               # Language (C, Perl, ...)
our $CLASS_DIR       = "/c/cs$CLASS";
our $PROJ_DIR        = "$CLASS_DIR/proj$PROJ";
our $TESTS_DIR       = "$PROJ_DIR/tests";
our $CLASS_BIN_DIR   = "$CLASS_DIR/bin";

# Common test functions and actions.
do "/home/accts/rh745/cs323/src/test_runner/runner.pl";

&limitCpuTime (2, 4);                           # Limit CPU-time per process
&limitWallClock (10);                           # Limit wall-clock per process
&limitFileSize (100000);                        # Limit size of files created
&limitHeapSize (1000000);                       # Limit size of heap
&limitProcesses (1000);                         # Limit #processes

my $total = 0;

&header ("Text");
$total += &runTest ("01", "No macro instructions");

&header ("Macroexpansion");
$total += &runTest ("02", "No # in VALUE");
$total += &runTest ("03", "# in VALUE");
$total += &runTest ("04", "Multiple # in VALUE");
$total += &runTest ("21", "VALUE contains macro that later changes");
$total += &runTest ("22", "foreach macro from specification");

&header ("Errors");
$total += &runTest ("05", "Missing {ARG} (error)");
$total += &runTest ("06", "{ARG} is not brace-balanced (error)");
$total += &runTest ("23", "Missing {NAME}/{VALUE}/{ELSE}/{FILENAME}");
$total += &runTest ("26", "MACRO not defined macro");
$total += &runTest ("27", "Attempt to def defined macro");

&header ("\\def");
$total += &runTest ("07", "Deeply nested braces in VALUE/ARG");

&header ("\\ifdef and \\if");
$total += &runTest ("08", "Name exists / argument nonempty");

&header ("\\undef");
$total += &runTest ("09", "Simple case");

&header ("\\expandafter");
$total += &runTest ("10", "Simple case (expand VALUE2 first)");
$total += &runTest ("11", "Example from specification");
$total += &runTest ("29", "BEFORE and expand(AFTER') are expanded together");
$total += &runTest ("34", "Expansion of AFTER defines macro used in BEFORE");

&header ("\\include");
$total += &runTest ("12", "Simple file with no macros");
$total += &runTest ("38", "Many \\includes (files closed?)");
$total += &runTest ("39", "\\def / \\macro extends beyond end of file");

&header ("Comments");
$total += &runTest ("13", "Ignore remainder of line and leading blanks/tabs");
$total += &runTest ("40", "Following line empty or only blanks/tabs");

&header ("Escaped characters");
$total += &runTest ("14", "Deleted if %, #, {, }, or \\, but not otherwise");
$total += &runTest ("46", '# vs. \# vs. \\\\# in VALUE');

&header ("Linear time and space");
$total += &runTest ("15", "Very short input with simple expansion and limited memory");
$total += &runTest ("16", "Very long input with no macros");
$total += &runTest ("17", "Very long macro NAME");
$total += &runTest ("49", "Very long VALUE field");

&header ("Is all storage reachable?");
$total += &runTest ("18", "Repeat of Tests #06 and #07");

print "\nEnd of Public Script\n";

printf ("\n%3d points Total for $NAME\n", $total);

#&header ("Non-credit Tests");

&sectionHeader ("POSSIBLE DEDUCTIONS");
print "These deductions are assessed after submission, as appropriate.\n\n";
&possibleDeduction ( -5, "Does not make");
&possibleDeduction ( -1, "Makefile missing");
&possibleDeduction ( -1, "Makefile incorrect");
&possibleDeduction ( -1, "Compilation errors using -Wall -std=c11 -pedantic");

&cleanupTesting ();
exit $total;

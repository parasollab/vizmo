#!/usr/bin/perl

use Getopt::Std;
getopts('c:d:');

#
# configuration
#
$outputdir = "/tmp/vizmo_nightly_logs";
$ADMIN_MAILTO = "jdenny\@cs.tamu.edu";
$workdir  = "/scratch/zenigata/jdenny/vizmo_nightly";

#
# find out which platform is used
#
if (!defined $opt_c || !(($opt_c eq "LINUX_gcc") || ($opt_c eq "LINUX_32_gcc"))) {
  die "must define compilation platform (-c LINUX_gcc | LINUX_32_gcc)";
}
if (!defined $opt_d || !(($opt_d eq "0") || ($opt_d eq "1"))) {
  die "must define debugging option (-d 0 | 1)";
}
$PLATFORM = $opt_c;
$DEBUG = $opt_d;

#
# setup shell variables
#
if ($opt_c eq "LINUX_gcc") {
  $GCC_PATH = "/usr/lib64/ccache";
  $MYENV    = "/usr/local/bin:$GCC_PATH:/usr/bin:/usr/X11R6/bin";
}
else {
  die "no suitable combination found";
}
$ENV{'PATH'} = $MYENV.":".$ENV{'PATH'};

#
# figure out time and date
#
@months   = qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);
@weekdays = qw(Sun Mon Tue Wed Thu Fri Sat);
($second, $minute, $hour, $dayOfMonth, $monthOfYear, $yearOffset, $dayOfWeek, $dayOfYear, $daylightSavings) = localtime();
$day      = $weekdays[$dayOfWeek];
$month    = $months[$monthOfYear];
$year     = 1900 + $yearOffset;
$fulldate = "$day-$month-$dayOfMonth-$year";

#
# check out code, compile and run tests
#
$vizmodir = "vizmo-$opt_c-$opt_d";
chdir $workdir or die "Can't cd to $workdir $!\n";
$OUTPUT = "platform = $PLATFORM / debug = $DEBUG\n";
$OUTPUT = $OUTPUT.`rm -rf $vizmodir 2>&1`;
$OUTPUT = $OUTPUT.`svn --quiet checkout svn+ssh://parasol-svn.cs.tamu.edu/research/parasol-svn/svnrepository/vizmo/trunk $vizmodir 2>&1`;
chdir "$workdir/$vizmodir/src";
$ENV{'PWD'} = "$workdir/$vizmodir/src";
$OUTPUT = $OUTPUT."Started at ".`date 2>&1`;
$OUTPUT = $OUTPUT."g++ path: ".`which g++ 2>&1`;
$OUTPUT = $OUTPUT."g++ details:\n".`g++ -c -v 2>&1`;
$OUTPUT = $OUTPUT.`make platform=$PLATFORM debug=$DEBUG reallyreallyclean 2>&1`;
$OUTPUT = $OUTPUT.`make platform=$PLATFORM debug=$DEBUG -j4 2>&1`;
if (-e "$workdir/$vizmodir/src/vizmo++") {
  $OUTPUT = $OUTPUT."=====\nPassed: vizmo compilation\n=====\n";
} else {
  $OUTPUT = $OUTPUT."=====\nFailed: vizmo compilation\n=====\n";
}
$OUTPUT = $OUTPUT."Done at ".`date 2>&1`;

#
# output log to /tmp
#
$ENV{'PATH'} = '/usr/local/bin/:/usr/X11R6/bin/:'.$ENV{'PATH'};
$ENV{'DISPLAY'} = '';

if (!-e "$outputdir") {
  `mkdir $outputdir`;
}
if (!-e "$outputdir/$fulldate") {
  `mkdir $outputdir/$fulldate`;
}
$outfile = "$outputdir/$fulldate/vizmo.$opt_c.debug$opt_d.out";
open(OUT, ">$outfile" || die "error, could not open $outfile for reading: $!");
print OUT $OUTPUT;
close(OUT);

#
# copy the output to the webserver
#

#
# send mail
#
open(MAIL, "|/usr/lib/sendmail -t");
print MAIL "To: $ADMIN_MAILTO\n";
print MAIL "From: vizmo_nightly\@tamu.edu\n";
print MAIL "Subject: Vizmo nightly - zenigata.cse.tamu.edu vizmo.$opt_c.debug$opt_d\n";
print MAIL "Done, output written to $outfile\n";
close(MAIL);


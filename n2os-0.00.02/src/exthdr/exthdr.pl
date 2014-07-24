#! /usr/bin/perl
##
##
##
no warnings;
my $currdir = `pwd`;
my $extsdir = $0;
$extsdir =~ s/(.*)\/.*$/$1/;
chomp($currdir);
chomp($extsdir);
foreach my $path(@ARGV) {
    $fname = $path;
    $fname =~ s/^.*[\/\\]//;
    my $gpath = `readlink -f $path`;
    chomp($gpath);
    chdir $extsdir;
    system("ln -sf $gpath $fname");
    printf("ln -sf $gpath $fname\n");
    chdir $currdir;
}

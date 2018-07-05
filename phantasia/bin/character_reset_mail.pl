#!/usr/bin/perl

open DATAFILE, ">/tmp/$ARGV[2].mail" or exit(1);

print DATAFILE "As you requested, the password has been reset for the Phantasia character\nnamed $ARGV[0].  Here is the new password.  Remember that it is case\nsensitive:\n\n";

print DATAFILE "character name: $ARGV[0]\n";
print DATAFILE "password: $ARGV[1]\n\n";

print DATAFILE "This password can be changed at http://www.phantasia4.com.  Login in to\nyour account and press \"Characters\" and then \"Change Pass\".\n\n";

print DATAFILE "If you have any questions, please contact the game moderator at\nbkelly\@dimensional.com.\n\n";

print DATAFILE "May your sword be swift and the evil fall quickly.\n";

close DATAFILE;

system ("mail -s \"Character $ARGV[0] Password Reset\" $ARGV[2] < /tmp/$ARGV[2].mail");

system("rm /tmp/$ARGV[2].mail");

exit(0);


#!/usr/bin/perl

open DATAFILE, ">/tmp/$ARGV[2].mail" or exit(1);

print DATAFILE "As you requested, the password has been reset for the Phantasia account\nnamed $ARGV[0].  Here is the new password and remember it is\ncase sensitive:\n\n";

print DATAFILE "account: $ARGV[0]\n";
print DATAFILE "password: $ARGV[1]\n\n";

print DATAFILE "This password can be changed at http://www.phantasia4.com.  Log in to your\naccount and press \"Account\" and then \"Change Pass\".\n\n";

print DATAFILE "If you have any questions, please contact the game moderator at\nbkelly\@dimensional.com.\n\n";

print DATAFILE "May the minions tremble at the mention of your name.\n";

close DATAFILE;

system ("mail -s \"Account $ARGV[0] Password Reset\" $ARGV[2] < /tmp/$ARGV[2].mail");

system("rm /tmp/$ARGV[2].mail");

exit(0);


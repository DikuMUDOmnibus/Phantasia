#!/usr/bin/perl

open DATAFILE, ">/tmp/$ARGV[2].mail" or exit(1);

print DATAFILE "Congratulations!  You have sucessfully created a new account on Phantasia\nat http://www.phantasia4.com. May all your adventures in the realm be\nfun and profitable.\n\n";

print DATAFILE "Here is your account information and confirmation code:\n\n";

print DATAFILE "account: $ARGV[0]\n";
print DATAFILE "confirmation code: $ARGV[1]\n\n";

print DATAFILE "To use your account, just go to http://www.phantasia4.com, click on\n\"The Game\", choose \"Java 1.18 Phantasia Client\" and enter your\naccount, password when confirmation code.  Remember, the password and\nconfirmation code are case sensitive.\n\n";

print DATAFILE "If you ever forget your account password, you can request it be changed\n by using the \"New Pass\" button from the login options.  A new\npassword will be generated and e-mailed to this address.\n\n";

print DATAFILE "Please keep your account and character passwords private.  If a\nplayer logs on with your account or your account's characters and acts\nin a manner that deserves to be banned, you will be banned.\n\n";

print DATAFILE "If you did not request or want an account on Phantasia, you don't\nneed to do anything.  Unless the above confirmation code is used to\nactivate the account, no more information will be sent to you.  The\nonly information kept is your e-mail address so it is not used again.\n\n";

print DATAFILE "If you have any questions, please contact the game moderator at\nbkelly\@dimensional.com.\n\n";

print DATAFILE "Good luck and happy hunting!\n";

close DATAFILE;

system ("mail -s \"Phantasia 4 Account Information\" $ARGV[2] < /tmp/$ARGV[2].mail");

system("rm /tmp/$ARGV[2].mail");

exit(0);


#!/usr/bin/perl

# Set the nvpairs array to the cookie information
@nvpairs=split(/; /, $ENV{HTTP_COOKIE});

# Break the array into a hash
foreach $pair (@nvpairs) {
        ($name, $value) = split(/=/, $pair);
        $cookie{$name} = $value;
}

# If there are no cookies, set them to 0
if (!defined($cookie{'num'})) {
    $cookie{'num'} = 0;
}

if (!defined($cookie{'hash'})) {
    $cookie{'hash'} = 0;
}

print "Content-type: text/html\n";

# Make sure the passed date is reasonable
$theTime = $ENV{'QUERY_STRING'};
$theTime = time - $theTime;
if ($theTime > 3600 or $theTime < 0) {
    print "Location: http://www.phantasia4.com\n\n";
    exit 0;
}

print "\n";

print "<HTML><HEAD><TITLE>Phantasia</TITLE></HEAD>";
print "<BODY BACKGROUND=\"/graphics/mis6b.jpg\" TEXT=\"#BBBB66\" LINK=\"#996699\" ALINK=\"#BB3333\" VLINK=\"#BB3333\">";
print "<TABLE WIDTH=\"100%\" CELLPADDING=\"0\"><TR><TD WIDTH=90></TD><TD>";

print "<CENTER><IMG SRC=\"/graphics/118_client.jpeg\" ALT=\"\" WIDTH=\"224\" HEIGHT=\"17\"></CENTER>";

print "<CENTER><P>The Java 1.18 client is now loading and should appear in a moment";

print "<APPLET CODE=\"pClient.class\" CODEBASE=\"/118_client\" ARCHIVE=\"118_client.jar\" HEIGHT=0 WIDTH=0><PARAM name=\"num\" value=\"$cookie{'num'}\"><PARAM name=\"hash\" value=\"$cookie{'hash'}\"><PARAM name=\"time\" value=\"$ENV{'QUERY_STRING'}\">";

print "<P>Your browser does not seem to support Java.  Please download a Java capable browser and try again.</P></APPLET></P>";
print "<P><B>Do not bookmark this page!</B></P><HR><BR>";

print "<iframe src=\"http://leader.linkexchange.com/1/X1422082/showiframe?\" width=468 height=60 marginwidth=0 marginheight=0 hspace=0 vspace=0 frameborder=0 scrolling=no><a href=\"http://leader.linkexchange.com/1/X1422082/clickle\" target=\"_top\"><img width=468 height=60 border=0 ismap alt=\"\" src=\"http://leader.linkexchange.com/1/X1422082/showle?\"></a></iframe><br><a href=\"http://leader.linkexchange.com/1/X1422082/clicklogo\" target=\"_top\"><img src=\"http://leader.linkexchange.com/1/X1422082/showlogo?\" width=468 height=16 border=0 ismap alt=\"\"></a><BR><HR>";

print "<P><A HREF=\"/index.html\">Home</A> | <A HREF=\"game.cgi\">The Game</A> | <A HREF=\"/intro.html\">The Rules</A> | <A HREF=\"/info.html\">Game Info</A></P>";

print "</TD></TR></TABLE></BODY></HTML>";

exit 0;

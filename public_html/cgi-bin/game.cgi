#!/usr/bin/perl

use Digest::MD5;

$theFile = "/home/bkelly/conf/cookie";

# Set the nvpairs array to the cookie information
@nvpairs=split(/; /, $ENV{HTTP_COOKIE});

# Break the array into a hash
foreach $pair (@nvpairs) {
        ($name, $value) = split(/=/, $pair);
        $cookie{$name} = $value;
}

print "Content-type: text/html", "\n";

# If either the cookies are gone, make a new set
if (!defined($cookie{'num'}) or !defined($cookie{'hash'})) {

    if (open theData, $theFile and $count = <theData>) {
	++$count;
	$cookie{'num'} = $count;
    }
    else {
        $count = 1;
	$cookie{'num'} = $count;
    }

    system("echo $count > $theFile\n");

# Calculate an MD5 hash with this machine number
    $ctx = Digest::MD5->new;
    $ctx->add("SecretWord");
    $ctx->add($count);
    $hash = $ctx->hexdigest;

    local(@days) = ("Sun","Mon","Tue","Wed","Thu","Fri","Sat");
    local(@months) = ("Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec");
    local($seconds,$min,$hour,$mday,$mon,$year,$wday) = gmtime(time + 31536000);

# formatting of date variables
    $seconds = "0" . $seconds if $seconds < 10;
    $min = "0" . $min if $min < 10; 
    $hour = "0" . $hour if $hour < 10; 
    $year += 1900; 

    $expires =
	    "$days[$wday], $mday-$months[$mon]-$year $hour:$min:$seconds GMT";

    print "Set-Cookie: num=$count; path=/; expires=$expires\n";
    print "Set-Cookie: hash=$hash; path=/; expires=$expires\n";
}

print "\n";

print "<HTML><HEAD><TITLE>Phantasia</TITLE></HEAD>";
print "<BODY BACKGROUND=\"/graphics/mis6b.jpg\" TEXT=\"#BBBB66\" LINK=\"#996699\" ALINK=\"#BB3333\" VLINK=\"#BB3333\">";
print "<TABLE WIDTH=\"100%\" CELLPADDING=\"0\"><TR><TD WIDTH=90></TD><TD>";

print "<CENTER><IMG SRC=\"/graphics/clients.jpeg\" ALT=\"\" WIDTH=\"196\" HEIGHT=\"22\"></CENTER>";
print "<P>Please choose the phantasia client you'd like to play on and click on it's name below.  Macintosh Netscape users can only use the 1.02 client.  I recommend the 1.18 client for everyone else.</P>";

print "<CENTER><TABLE WIDTH=\"100%\" CELLPADDING=\"0\"><TR ALIGN=CENTER><TD>";
print "<A HREF=\"102_client.cgi?", time, "\"><IMG SRC=\"/graphics/102_client.jpeg\" ALT=\"\" WIDTH=\"224\" HEIGHT=\"17\"></A></TD><TD>";
print "<A HREF=\"118_client.cgi?", time, "\"><IMG SRC=\"/graphics/118_client.jpeg\" ALT=\"\" WIDTH=\"224\" HEIGHT=\"17\"></A></TD></TR></TABLE><HR><BR>";

print "<P>theFlower has created a cool post calculator for Windows.  Click <A HREF=\"/posts.zip\">here</A> to download a copy. <BR>(Updated 12/16/01) </P>";
   
print "<P>A handy monster tool from KillerX for Windows can now be downloaded <A HREF=\"/monsters.zip\">here</A>. <BR>(Updated 01/06/02) </P><HR><BR>";

print "<iframe src=\"http://leader.linkexchange.com/1/X1422082/showiframe?\" width=468 height=60 marginwidth=0 marginheight=0 hspace=0 vspace=0 frameborder=0 scrolling=no><a href=\"http://leader.linkexchange.com/1/X1422082/clickle\" target=\"_top\"><img width=468 height=60 border=0 ismap alt=\"\" src=\"http://leader.linkexchange.com/1/X1422082/showle?\"></a></iframe><br><a href=\"http://leader.linkexchange.com/1/X1422082/clicklogo\" target=\"_top\"><img src=\"http://leader.linkexchange.com/1/X1422082/showlogo?\" width=468 height=16 border=0 ismap alt=\"\"></a><BR><HR>";

print "<P><A HREF=\"/index.html\">Home</A> | The Game | <A HREF=\"/intro.html\">The Rules</A> | <A HREF=\"/info.html\">Game Info</A> </P>";

print "</CENTER></TD></TR></TABLE></BODY></HTML>";

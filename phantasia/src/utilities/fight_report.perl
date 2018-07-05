#!/usr/bin/perl -w

# this program is desigined to parse the output of the combat data file
# and give the information in a standard report.

# Modifications
# Starting programming - 12/28/99			 Brian Kelly

# Configurables
$dataFile = "/var/phantasia/combat";

# Initialize variables
foreach $class ("Magic-User", "Fighter", "Elf", "Dwarf", "Hafling",
		"Experimento") {

	$melee{$class}[0] = 0;
	$melee{$class}[1] = 0;
	$skirmish{$class}[0] = 0;
	$skirmish{$class}[1] = 0;
	$nick{$class}[0] = 0;
	$nick{$class}[1] = 0;
	$allornothing{$class}[0] = 0;
	$allornothing{$class}[1] = 0;
	$magicbolt{$class}[0] = 0;
	$magicbolt{$class}[1] = 0;
	$forceshield{$class} = 0;
	$increasemight{$class} = 0;
	$invisible{$class} = 0;
	$transform{$class} = 0;
	$transport{$class} = 0;
}

# Main loop

# Open the combat log file
open(theFile, "$dataFile") or die "Unable to open $dataFile\n";

# Read each line of the file and record the information
while (defined($line = <theFile>)) {
	chomp($line);
	@theData = split(/\s+/, $line);
	if ($theData[6] eq "melee") {
		++$melee{$theData[7]}[0];
		$melee{$theData[7]}[1] += $theData[8];
	}
	elsif ($theData[6] eq "skirmish") {
		++$skirmish{$theData[7]}[0];
		$skirmish{$theData[7]}[1] += $theData[8];
	}
	elsif ($theData[6] eq "nick") {
		++$nick{$theData[7]}[0];
		$nick{$theData[7]}[1] += $theData[8];
	}
	elsif ($theData[6] eq "allornothing") {
		++$allornothing{$theData[7]}[0];
		$allornothing{$theData[7]}[1] += $theData[8];
	}
	elsif ($theData[6] eq "magicbolt") {
		++$magicbolt{$theData[7]}[0];
		$magicbolt{$theData[7]}[1] += $theData[8];
	}
	elsif ($theData[6] eq "forceshield") {
		++$forceshield{$theData[7]};
	}
	elsif ($theData[6] eq "increasemight") {
		++$increasemight{$theData[7]};
	}
	elsif ($theData[6] eq "invisible") {
		++$invisible{$theData[7]};
	}
	elsif ($theData[6] eq "transform") {
		++$transform{$theData[7]};
	}
	elsif ($theData[6] eq "transport") {
		++$transport{$theData[7]};
	}
	else {
		print "$theData[6]\n";
	}
}


# Print out the information
$uses = 0;
$damage = 0;

foreach $class (keys %melee) {
	$uses += $melee{$class}[0];
	$damage += $melee{$class}[1];
	print ("$class meleed $melee{$class}[0] times for $melee{$class}[1] damage.\n");
}	

print ("Total of $uses melees for $damage damage.\n\n");


$uses = 0;
$damage = 0;

foreach $class (keys %skirmish) {
	$uses += $skirmish{$class}[0];
	$damage += $skirmish{$class}[1];
	print ("$class skirmished $skirmish{$class}[0] times for $skirmish{$class}[1] damage.\n");
}	

print ("Total of $uses skirmishes for $damage damage.\n\n");


$uses = 0;
$damage = 0;

foreach $class (keys %nick) {
	$uses += $nick{$class}[0];
	$damage += $nick{$class}[1];
	print ("$class nicked $nick{$class}[0] times for $nick{$class}[1] damage.\n");
}	

print ("Total of $uses nicks for $damage damage.\n\n");


$uses = 0;
$damage = 0;

foreach $class (keys %allornothing) {
	$uses += $allornothing{$class}[0];
	$damage += $allornothing{$class}[1];
	print ("$class allornothinged $allornothing{$class}[0] times for $allornothing{$class}[1] damage.\n");
}	

print ("Total of $uses allornothings for $damage damage.\n\n");


$uses = 0;
$damage = 0;

foreach $class (keys %magicbolt) {
	$uses += $magicbolt{$class}[0];
	$damage += $magicbolt{$class}[1];
	print ("$class magicbolted $magicbolt{$class}[0] times for $magicbolt{$class}[1] damage.\n");
}	

print ("Total of $uses magicbolts for $damage damage.\n\n");


$uses = 0;

foreach $class (keys %forceshield) {
	$uses += $forceshield{$class};
	print ("$class forceshielded $forceshield{$class} times.\n");
}	

print ("Total of $uses forceshields.\n\n");


$uses = 0;

foreach $class (keys %increasemight) {
	$uses += $increasemight{$class};
	print ("$class increasemight $increasemight{$class} times.\n");
}	

print ("Total of $uses increasemight.\n\n");


$uses = 0;

foreach $class (keys %invisible) {
	$uses += $invisible{$class};
	print ("$class invisible $invisible{$class} times.\n");
}	

print ("Total of $uses invisible.\n\n");


$uses = 0;

foreach $class (keys %transform) {
	$uses += $transform{$class};
	print ("$class transform $transform{$class} times.\n");
}	

print ("Total of $uses transform.\n\n");


$uses = 0;

foreach $class (keys %transport) {
	$uses += $transport{$class};
	print ("$class transport $transport{$class} times.\n");
}	

print ("Total of $uses transport.\n\n");



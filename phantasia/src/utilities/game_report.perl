#!/usr/bin/perl -w

# this program is desigined to parse the output of the combat data file
# and give the information in a standard report.

# Modifications
# Starting programming - 12/28/99			 Brian Kelly

# Configurables
$dataFile = "/tmp/all.games";

# Initialize variables
foreach $class ("Magic-User", "Fighter", "Elf", "Dwarf", "Hafling",
		"Experimento") {

	$death{$class}[0] = 0;
	$death{$class}[1] = 0;
}

# Main loop

# Open the combat log file
open(theFile, "$dataFile") or die "Unable to open $dataFile\n";

# Read each line of the file and record the information
while (defined($line = <theFile>)) {
	chomp($line);
	@theData = split(/\s+/, $line);
	if ($theData[6] eq "died") {
		++$death{$theData[7]}[0];
		$death{$theData[7]}[1] += $theData[8];
	}
	else {
		$extras{$theData[6]} = 1;
	}
}


# Print out the information
$uses = 0;
$damage = 0;

foreach $class (keys %death) {
	$uses += $death{$class}[0];
	$damage += $death{$class}[1];
	if ($death{$class}[0] > 0) {
	    $average = $death{$class}[1]/$death{$class}[0];
	}
	else {
	    $average = 0;
	}
	print ("$class killed $death{$class}[0] times - average level = $average.\n");
}	

print ("Total of $uses deaths for $damage levels lost.\n\n");

print ("Here are unused headers:\n");
foreach $header (keys %extras) {
	print ("$header\n");
}

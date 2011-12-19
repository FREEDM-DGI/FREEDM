#!/usr/bin/perl -w

# file:     configure.pl
# brief:    automated freedm.cfg generator
# author:   Thomas Roth <tprfh7@mst.edu>

if( $#ARGV < 0 || $#ARGV > 1 )
{
    print "Usage: $0 <Hostname File> [DGI Number(optional)]\n";
    exit;
}

# default number of DGI
local $NUM_DGI = 3;

# empty the hostname tokens
local @TOKEN    = ();
local @HOST     = ();
local @PORT     = ();

# initialize the local variables
&InitializeHostToken($ARGV[1]);

# read the user input file
&ReadHostFile($ARGV[0]);
&ValidateUserInput($ARGV[0]);

# retrieve the DGI instance
my $instance = &ThisHostIndex();

# create and write the freedm.cfg file
my $string = &CreateFreedmConfig($instance);
&WriteStringAsFile($string,"src/freedm.cfg");


# ----- FUNCTIONS ----------
# brief:    sets the token variable, initializes hostname and port variables
# pre:      none
# post:     @TOKEN is initialized
# param:    0, number of dgi (optional)
# error:    invalid number of dgi nodes
sub InitializeHostToken
{
    # get optional parameters
    my $ndgi = ( defined $_[0] ? $_[0] : $NUM_DGI );
    
    # check for non-integer user input
    if( $ndgi !~ /^[1-9]\d*$/ )
    {
        print "ERROR: Invalid number of DGI as argument, $ndgi\n";
        exit 1;
    }
    
    # initialize the hostname tokens
    push(@TOKEN,"PSCAD");
    for( my $i = 1; $i <= $ndgi; $i++ )
    {
        push(@TOKEN,"DGI-$i");
    }
    
    # initialize the hostname values
    @HOST = ('') x ($ndgi+1);
    @PORT = ('') x ($ndgi+1);
}

# brief:    reads the host file and sets the hostname / port variables
# pre:      none
# post:     ReadFileAsString(filepath) is invoked
# post:     @HOST and @PORT are initialized from the file content
# param:    0, filepath of the input file to read
sub ReadHostFile
{
    # get parameters
    my $filepath = $_[0];

    # read the hostname file
    my $hostfile = &ReadFileAsString($filepath);
    
    # process each hostname token
    for( my $i = 0; $i <= $#TOKEN; $i++ )
    {
        ($HOST[$i],$PORT[$i]) = &GetHostnamePort($TOKEN[$i],$hostfile);
    }
}

# brief:    extracts a hostname and port number based on an initial token
# pre:      string entries take the form 'token = hostname:port'
# post:     CheckHostnamePort(hostname,port) is invoked
# param:    0, the token that identifies the hostname:port combination
# param:    1, the string that contains the token entry
# return:   (hostname,port) of the extracted entry
sub GetHostnamePort
{
    # get parameters
    my $token   = $_[0];
    my $string  = $_[1];
    my $hostname;
    my $port;
    
    # match 'Token = Hostname:Port' where # denotes a comment
    #   [ \f\r\t]*  match any amount of non-newline whitespace
    #   [^#\n]+     match at least one non-comment non-newline
    if( $string =~ /^[ \f\r\t]*$token[ \f\r\t]*=[ \f\r\t]*([^#\n]+):([^#\n]+)/mi )
    {
        ($hostname,$port) = CheckHostnamePort($1,$2);
    }
    
    return($hostname,$port);
}

# brief:    verifies the content of a hostname and port number combination
# pre:      none
# post:     warning message displayed if the check fails
# param:    0, the hostname to check
# param:    1, the port number to check
# return:   (hostname,port) where an entry is set to empty if the check fails
sub CheckHostnamePort
{
    # get parameters
    my $hostname    = $_[0];
    my $port        = $_[1];
    
    # check for invalid hostnames given the convention:
    #   cannot begin or end with a hyphen
    #       ^(?!-)      cannot begin with -
    #       (?<!-)$     cannot end with -
    #   alpha-numeric hyphenated string
    #       [A-Z\d-]    i for case-insensitive
    #   maximum length of 63 characters
    #       {1,63}      1 to 63 chars
    if( $hostname !~ /^(?!-)[A-Z\d\-\.]{1,63}(?<!-)$/i )
    {
        print "WARNING: Invalid Hostname, $hostname\n";
        $hostname = '';
    }
    
    # check for invalid port numbers outside the range (0,65535]
    unless( $port =~ /^\d{1,5}$/ && $port > 0 && $port <= 65535 )
    {
        print "WARNING: Invalid Port Number, $port\n";
        $port = '';
    }
    
    return($hostname,$port);
}

# brief:    validates @HOST and @PORT for existence and uniqueness
# pre:      @HOST and @PORT must both contain valid user data
# post:     none
# param:    0, filepath of the input file that generated @HOST and @PORT
# error:    @HOST or @PORT entry in range 0...$#TOKEN is empty
# error:    @HOST entry in range 1...$#TOKEN is a duplicate
sub ValidateUserInput
{
    # get parameters
    my $filepath = $_[0];
    my $valid_input = 1;
    my %count = ();
    
    # check each hostname entry
    for( my $i = 0; $i <= $#TOKEN; $i++ )
    {
        if( !$HOST[$i] || !$PORT[$i] )
        {
            print "ERROR: $TOKEN[$i] corrupt in file $filepath\n";
            undef $valid_input;
        }
        else
        {
            # increase the hostname instance count
            $count{$HOST[$i]}++ if( $i != 0 );
        }
    }
    
    # check for duplicate hostname entries
    foreach my $host (keys %count)
    {
        if( $count{$host} > 1 )
        {
            print "ERROR: Duplicate hostname $host in $filepath\n";
            undef $valid_input;
        }
    }
    
    # break on invalid input
    if( !defined $valid_input )
    {
        print "Script failed due to incomplete input file $filepath\n";
        exit 1;
    }
}

# brief:    determines the token index of this machine based on hostname
# pre:      @HOST must contain the hostname of this machine
# post:     system 'hostname' command is invoked
# return:   index reference to @TOKEN for this machine
# error:    hostname does not match a @HOST entry
sub ThisHostIndex
{
    # read the machine hostname
    my $hostname = `"hostname"`;
    my $index = '';
    
    # trim the whitespace from hostname
    $hostname =~ s/^\s+//;
    $hostname =~ s/\s+$//;
    
    # search the hostname array
    for( my $i = 1; $i <= $#TOKEN; $i++ )
    {
        if( $HOST[$i] eq $hostname )
        {
            # set the host index
            $index = $i;
        }
    }
    
    # check for index existence
    if( !$index )
    {
        print "ERROR: This hostname $hostname not in configuration file\n";
        exit 1;
    }
    
    return $index;
}

# brief:    generates a string version of freedm.cfg from local variables
# pre:      @HOST and @PORT must be initialized to valid data
# pre:      passed index must reference an entry in @TOKEN
# post:     none
# param:    0, @TOKEN index for the machine running the script
# return:   string version of freedm.cfg
sub CreateFreedmConfig
{
    # get parameters
    my $instance = $_[0];
    my $string = '';
    
    # set the file header
    $string .= "# This file was generated by the script $0\n";
    $string .= "# Script Author: Thomas Roth <tprfh7\@mst.edu>\n";
    $string .= "\n";
    
    # add each dgi peer (self included)
    $string .= "# Add hostnames to the peer list\n";
    for( my $i = 1; $i <= $#TOKEN; $i++ )
    {
        $string .= &AddHost($i);
    }
    $string .= "\n";
    
    # add host-specific information
    $string .= &AddSelf($instance);
    $string .= "\n";
    
    # add the simulation server
    $string .= &LineClient($instance);
    $string .= "\n";
    
    return $string;
}

# brief:    generates a single peer entry for freedm.cfg
# pre:      passed index must reference an entry in @TOKEN
# post:     none
# param:    0, valid @TOKEN index of the peer to add
# return:   a single add-host entry for freedm.cfg
sub AddHost
{
    # get parameters
    my $index = $_[0];

    return "add-host=$HOST[$index]:$PORT[$index]\n";
}

# brief:    generates the device entries for freedm.cfg
# pre:      passed index must reference an entry in @TOKEN
# post:     none
# param:    0, @TOKEN index for the machine running the script
# return:   address, port, and device entries for freedm.cfg
sub AddSelf
{
    # get parameters
    my $index = $_[0];
    my $string = '';
    
    $string .= "# Listen on any interface with the given TCP port\n";
    $string .= "address=0.0.0.0\n";
    $string .= "port=$PORT[$index]\n";
    $string .= "\n";
    $string .= "# Use one SST device with this DGI's unique identifier\n";
    $string .= "add-device=SST$index\n";
    $string .= "\n";
    $string .= "# Set the debug output level (0 = minimal, 7 = maximal)\n";
    $string .= "verbose 5\n";
    
    return $string;
}

# brief:    generates the lineclient entries for freedm.cfg
# pre:      $HOST[0] and $PORT[0] must be set to the lineclient values
# pre:      passed index must reference an entry in @TOKEN
# post:     none
# param:    0, @TOKEN index for the machine running the script
# return:   lineclient entries for freedm.cfg
sub LineClient
{
    # get parameters
    my $index = $_[0];
    my $string = '';
    
    # calculate the numeric port number
    my $port = $PORT[0] + $index;
    
    $string .= "# Hostname of the lineclient simulation server\n";
    $string .= "lineclient-host=$HOST[0]\n";
    $string .= "# TCP port number of the lineclient simulation server\n";
    $string .= "lineclient-port=$port\n";
    
    return $string;
}

# brief:    read the content of the passed file as a string
# pre:      user must have read access to the destination filepath
# post:     none
# param:    0, the filepath of the source to read
# return:   string value of the passed file
# error:    unable to open the file
sub ReadFileAsString
{
    # get parameters
    my $filepath = $_[0];
    my $string;
    
    # read file into string
    open INFILE, $filepath or die "Failed to open $filepath: $!";
    {
        $string = do { local $/; <INFILE> };
    } close INFILE;

    return $string;
}

# brief:    overwrite the content of a file with the passed string
# pre:      user must have write access to the destination filepath
# post:     file is created / replaced with the passed string content
# param:    0, the content of the file to create
# param:    1, the filepath of the destination
# error:    unable to open the file
sub WriteStringAsFile
{
    # get parameters
    my $content = $_[0];
    my $filepath = $_[1];
    
    # open the output file
    open OUTFILE, ">$filepath" or die "Failed to open $filepath: $!";
    {
        print OUTFILE $content;
    }
    close OUTFILE;
}

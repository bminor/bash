#From: jrmartin@rainey.blueneptune.com (James R. Martin)
#Newsgroups: comp.unix.shell
#Subject: Re: testing user input on numeric or character value
#Date: 26 Nov 1997 01:28:43 GMT

# isnum returns True if its argument is a valid number,
# and False (retval=1) if it is any other string.
# The first pattern requires a digit before the decimal
# point, and the second after the decimal point.

# BASH NOTE: make sure you have executed `shopt -s extglob' before
# trying to use this function, or it will not work

function isnum # string
{
    case $1 in
    ?([-+])+([0-9])?(.)*([0-9])?([Ee]?([-+])+([0-9])) )
        return 0;;
    ?([-+])*([0-9])?(.)+([0-9])?([Ee]?([-+])+([0-9])) )
        return 0;;
    *) return 1;;
    esac
}

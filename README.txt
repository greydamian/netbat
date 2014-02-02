netbat
======

Overview
--------

Netbat is a simple TCP client/server utility program.

Netbat can act as either a TCP client or TCP server. Data is concurrently read 
from standard input (stdin) then written to the TCP stream and read from the 
TCP stream and written to standard output (stdout).

Netbat terminates execution normally when it receives end-of-transmission from 
the TCP stream.

My motivation for writing netbat was to better understand and practice socket 
and thread programming. It was heavily influenced by attempting to emulate some 
behaviours of the Netcat program (http://nc110.sourceforge.net/), which was 
originally developed by *Hobbit*, and is a much more fully-featured utility.

Compatibility
-------------

    * Linux
    * Unix
    * OS X

Installation
------------

Full installation of netbat is a 2 step process. However, the second step is 
optional.

Firstly, navigate to the src directory and run the following command:

    $ bash build.sh

Once this has completed, you can run netbat by navigating to the bin directory 
and running the following command:

    $ ./netbat

Secondly, and optionally, you can install netbat system-wide by copying the 
contents of the bin directory into your path. For example, you could execute a 
command such as:

    # cp bin/* /usr/local/bin/

Examples
--------

There are many purposes for which netbat can be used. However, here I will 
focus on 3 specific usage examples: file transfer, HTTP request, instant 
messaging.

Netbat can be used to transer a file (e.g. myfile) by executing the following 
commands on their respective hosts:

    host-a$ netbat 4444 < myfile
    host-b$ netbat 192.168.0.2 4444 > myfile

N.B. The previous example presumes that host-a has the IP address 192.168.0.2.

Netbat can be used to send a HTTP request to a web server, and display the 
response, by executing the following command:

    $ echo -ne 'GET / HTTP/1.1\r\nHost: 192.168.0.1\r\n' \
      'Connection: close\r\n\r\n' | netbat 192.168.0.1 80

N.B. The previous example presumes the web server is running on port 80 of the 
host with IP address 192.168.0.1.

Netbat can be used as a simple instant messaging client by executing the 
following commands on their respective hosts:

    host-a$ netbat 4444
    host-b$ netbat 192.168.0.2 4444

After executing the previous commands, anything typed as input to one of the 
commands will be displayed as output from the other and vice versa. Supplying 
EOF (End Of File) to one of the programs will terminate the session, this can 
usually be done by typing [Ctrl] + [D].

N.B. The previous example presumes that host-a has the IP address 192.168.0.2.

License
-------

Copyright (c) 2014 Damian Jason Lapidge

Licensing terms and conditions can be found within the file LICENSE.txt.


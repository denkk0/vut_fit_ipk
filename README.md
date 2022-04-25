# IPK

Documentation of 1st project for IPK 2021/2022

Author: Denis Kramár (xkrama06) [xkrama06@vutbr.cz](mailto:xkrama06@vutbr.cz)

## 1. Description
The assignment was to create server, which would communicate over HTTP protocol to gather various information about the server.

## 2. Launching
To run the server, it is necessary to provide a port number. Example:

    ./hinfosvc 5656

## 3. Implementation details

First of all, the command line argument passed (_port number_) is verified. After that, the socket is created and its options are changed. `SO_REUSEADDR` allows to reuse the local addresses, `SO_REUSEPORT` allows load distribution in multi-threaded server. The socket is then bound to a port which is given while executing the program. In the end, the server is listening for incoming connection, accepting, and reading them. After reading the incoming message, an appropriate response message is sent back to the client and the socket is closed.
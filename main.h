#ifndef _MAIN_H
    #define _MAIN_H

    #include <iostream>
    #include <string>
    #include <vector>
    #include <cstring>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>

    using namespace std;

    enum errcodes {
        EOK = 0,
        EWRONGPARAMS,
        EPOPEN,
        EPCLOSE,
        ECREATESOCKET,
        EOPT,
        EBIND,
        ELISTEN,
        EACCEPT,
        ERECV,
        ESEND,
        ECLOSE
    };

    const vector<string> errmsg = {
        "", //EOK
        "Command line arguments missing or not numerical! \n",
        "Opening the pipe went wrong. \n",
        "Closing the pipe went wrong. \n",
        "Creating socket file descriptor failed. \n",
        "Changing the option state of a socket failed. \n",
        "Binding port to a socket failed. \n",
        "Listening for connections on a socket failed. \n",
        "Accepting a connection on a socket failed. \n",
        "Receiving a message from a socket failed. \n",
        "Sending a message onto a socket failed. \n",
        "Closing the socket file descriptor failed \n"
    };

    void errorOccurence(int eid);
    int checkParams(int argc, char *argv[]);
    int exec(string *cmd_result, const char* cmd);
    vector<unsigned long int> formatString(string str);
    float getCPUload(int s);
    int createSocket(int *sockfd);
    int sopt(int *sockfd, int opt, unsigned int state);
    int sbind(int *sockfd, unsigned int port);
    void server(int *sockfd, int backlog);

#endif // _MAIN_H
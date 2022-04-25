/** 
 * @file main.cpp
 * @author Denis Kramár <xkrama06@vutbr.cz>
 */

#include "main.h"

/**
 * Exit program with appropriate error message and error code.
 * 
 * @param eid error code
 */
void errorOccurence(int eid) {
    cerr << errmsg[eid];
    exit(eid);
}

/**
 * Check the parameters of the function. If function doesn't have argument,
 * which is numerical, an error will occur.
 * 
 * @param argc number of arguments
 * @param argv arguments
 * @return EOK, if no error occurs, EWRONGPARAMS otherwise
 */
int checkParams(int argc, char *argv[]) {
    if (argc != 2) {
        return EWRONGPARAMS; 
    }

    int i = 0;
    while (argv[1][i] != '\0') {
        if (!isdigit(argv[1][i])) {
            return EWRONGPARAMS;
        }
        i++;
    }

    return EOK;
}

/**
 * Execute cmd command
 * 
 * Executes cmd command. The output is stored in cmd_result variable, the 
 * command to be executed is passed via cmd variable.
 * 
 * @param cmd_result stores output of the command
 * @param cmd command to be executed
 * @return EOK if no error occurs, EPOPEN / EPCLOSE else
 */
int exec(string *cmd_result, const char* cmd) {
    char buffer[2048];

    auto pipe = popen(cmd, "r");
    if (!pipe) {
        return EPOPEN;
    };

    if (fgets(buffer, 2048, pipe) != nullptr) {
        *cmd_result += buffer;
    }

    auto rc = pclose(pipe);
    if (rc == -1) {
        return EPCLOSE;
    }
    
    return EOK;
}

/**
 * Format string into vector.
 * 
 * Input string of numbers separated by whitespace is split and formatted
 * into vector made of unsigned long ints.
 * 
 * @param str input string
 * @return formatted vector
 */
vector<unsigned long int> formatString(string str) {
    vector<unsigned long int> formatted_vector;
    string tmp = "";

	for (unsigned int i = 5; i < str.length(); i++) {
		if(str[i]==' '){
			formatted_vector.push_back(stol(tmp));
			tmp = "";
		}
		else{
			tmp += str[i];
		}
		
	}
	formatted_vector.push_back(stol(tmp));

    return formatted_vector;
}

/**
 * Outputs a CPU load.
 * 
 * Runs a cmd command to get CPU stats, then runs another after some period of
 * time. Using various formulas used, the CPU load is computed and outputted.
 * 
 * @param s period of time in seconds after which should be the other command 
 *          executed
 * @return cpu load in percentages
 */
float getCPUload(int s) {
    string prevCmdOutput;
    string cmdOutput;
    vector<unsigned long int> prevStat;
    vector<unsigned long int> stat;
    long long int prevIdle;
    long long int idle;
    long long int prevNonIdle;
    long long int nonIdle;
    long long int prevTotal;
    long long int total;
    long long int totald;
    long long int idled;
    double cpu_perc; 
    int ret;

    if ((ret = exec(&prevCmdOutput, "cat /proc/stat | head -n 1"))) {
        errorOccurence(ret);
    }

    sleep(s);

    if ((ret = exec(&cmdOutput, "cat /proc/stat | head -n 1"))) {
        errorOccurence(ret);
    }

    prevStat = formatString(prevCmdOutput);
    stat = formatString(cmdOutput);

    // PrevIdle = previdle + previowait
    // Idle = idle + iowait
    prevIdle = prevStat[3] + prevStat[4];
    idle = stat[3] + stat[4];

    // PrevNonIdle = prevuser + prevnice + prevsystem + previrq + prevsoftirq + prevsteal
    // NonIdle = user + nice + system + irq + softirq + steal
    prevNonIdle = prevStat[0] + prevStat[1] + prevStat[2] + prevStat[5] + prevStat[6] + prevStat[7];
    nonIdle = stat[0] + stat[1] + stat[2] + stat[5] + stat[6] + stat[7];

    // PrevTotal = PrevIdle + PrevNonIdle
    // Total = Idle + NonIdle
    prevTotal = prevIdle + prevNonIdle;
    total = idle + nonIdle;

    // totald = Total - PrevTotal
    // idled = Idle - PrevIdle
    totald = total - prevTotal;
    idled = idle - prevIdle;

    // CPU_Percentage = (totald - idled)/totald * 100
    cpu_perc = (double)(totald - idled)/(double)totald * 100;

    return cpu_perc;
}

/** 
 * Create a socket file descriptor
 * 
 * @param sockfd socket descriptor
 * @return EOK if no error occurs, ECREATESOCKET else
 */
int createSocket(int *sockfd) {
    *sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*sockfd < 0) {
        return ECREATESOCKET;
    }
    return EOK;
}

/**
 * Change the option state of a socket 
 * 
 * @param sockfd socket descriptor
 * @param opt option we want to change
 * @param state 1 enable, 0 disable
 * @return EOK if no error occurs, EOPT else
 */
int sopt(int *sockfd, int opt, unsigned int state) {
    if (setsockopt(*sockfd, SOL_SOCKET, opt, &state, sizeof(int))) {
        return EOPT;
    }

    return EOK;
}

/** 
 * Binds socket to a given port.
 * 
 * @param sockfd socket file descriptor
 * @param port port to be binded to a socket
 * @return EOK if no error occurs, EBIND else
 */
int sbind(int *sockfd, unsigned int port) {
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port); 
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(*sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        return EBIND;
    } 

    return EOK;
}

/**
 * Function server listens for connections on a socket, accepts the incoming
 * connection, and receives a message from socket. The message is then read,
 * and appropriate message is sent back to the client according to the endpoint.
 * The client socket is closed afterwards.
 * 
 * @param sockfd socket file descriptor
 * @param backlog number of backlogs
 */
void server(int *sockfd, int backlog) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    if (listen(*sockfd, backlog)) {
        errorOccurence(ELISTEN);
    }

    int clientSock, count;
    char ret[256];
    string ret_cut;
    while (true) {
        clientSock = accept(*sockfd, (struct sockaddr *)&addr, &addrlen);
        
        if ((clientSock) < 0) {
            errorOccurence(EACCEPT);
        } else {
            // clear string if it was accepted
            ret_cut = "";
        }

        if (recv(clientSock, ret, 256, 0) < 0) {
            errorOccurence(ERECV);
        } 


        // reset counter
        count = 0;
        for(unsigned int i = 0; i < 256; i++) {
            if (ret[i] == ' ') count++;
            if (count == 2) break;
            ret_cut += ret[i];
        }

        // send a message according to a endpoint
        string sendmsg = "";
        if (ret_cut == "GET /hostname") { 
            string hostname;
            exec(&hostname, "cat /proc/sys/kernel/hostname | tr -d '\n'");
            sendmsg += "HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n";
            sendmsg += hostname;
            sendmsg += "\r\n";
        } else if (ret_cut == "GET /cpu-name") {
            string cpuName; 
            exec(&cpuName, "cat /proc/cpuinfo | grep 'model name' | head -n 1 | cut -f 2 -d ':' | awk '{$1=$1}1' | tr -d '\n'");
            sendmsg += "HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n";
            sendmsg += cpuName;
            sendmsg += "\r\n";
        } else if (ret_cut == "GET /load") {
            char load[10];
            sprintf(load, "%.2f", getCPUload(1));
            sendmsg += "HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n";
            sendmsg += load;
            sendmsg += "%\r\n";
        } else {
            sendmsg += "HTTP/1.1 404 Not Found\r\n";
        }

        if (send(clientSock, sendmsg.c_str(), sendmsg.length(), 0) == -1) {
            errorOccurence(ESEND);
        }

        if(close(clientSock)) {
            errorOccurence(ECLOSE);
        }
    }
}

int main(int argc, char *argv[]) {
    int sockfd;
    int ret;

    if ((ret = checkParams(argc, argv))) {
        errorOccurence(ret);
    }

    if ((ret = createSocket(&sockfd))) {
        errorOccurence(ret);
    }

    if ((ret = sopt(&sockfd, SO_REUSEADDR, 1))) {
        errorOccurence(ret);
    }

    if ((ret = sopt(&sockfd, SO_REUSEPORT, 1))) {
        errorOccurence(ret);
    }

    if ((ret = sbind(&sockfd, stoi(argv[1])))) {
        errorOccurence(ret);
    }

    server(&sockfd, 1);

    return 0;
}

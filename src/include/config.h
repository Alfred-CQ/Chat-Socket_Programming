#ifndef _CONFIG_H_
    #define _CONFIG_H_

    #include <arpa/inet.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <string.h>
    #include <unistd.h>
    #include <string.h>

    #include <iostream>
    #include <thread>
    #include <ctime>
    #include <chrono>
    #include <vector>
    #include <map>
    #include <fstream>
    #include <utility>

    typedef unsigned int uint;
    typedef std::pair<int, int> OWNER_TURN;

    #define STR_LENGTH 256
    #define STR_LENGTH_MAX 999
    #define BYTES_PER_PACKET 150

    using std::cout; using std::cin; using std::endl;
    using std::string;
    using std::vector;
    using std::map;
    using std::pair;
    using std::thread;

#endif
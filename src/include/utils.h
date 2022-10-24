#ifndef _UTILS_H_
    #define _UTILS_H_

        #include <iostream>
        #include <string>
        using std::string;

        string complete_digits(int t, bool type)
        {
            if (type)
            {
                if (t < 10)
                    return ('0' + std::to_string(t));
                return std::to_string(t);
            }
            else
            {
                if (t < 10)
                    return ("00" + std::to_string(t));
                else if (t < 100)
                    return ('0' + std::to_string(t));
                return std::to_string(t);
            }
        }

        bool error_communication(int number_bytes)
        {
            if ( number_bytes <= 0)
            {
                std::cout << "[ ERROR ] -> Bytes\n"; 
                return true;
            }
            return false;
        }

#endif

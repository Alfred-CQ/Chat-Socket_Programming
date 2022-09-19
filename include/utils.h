#ifndef _UTILS_H_
    #define _UTILS_H_

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

#endif

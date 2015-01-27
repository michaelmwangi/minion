#ifndef CONSTANTS_H
#define CONSTANTS_H

enum OperationConsts{
    KiloByte_Size = 1024,
    Megabyte_Size = 1024 * 1024,
    GigaByte_Size = 1024 * 1024 * 1024
};

enum class OperationCode{
    None = 0,
    Okay = 200,
    Not_Found = 404,
    Server_Error = 505,
    Bad_Gateway = 502,
    Proxy_Config = 405,
    Resource_Forbidden = 403,
    Service_Unavailable = 503,
    Moved = 302
};

#endif // CONSTANTS_H


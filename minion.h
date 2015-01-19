#ifndef MINION
#define MINION
#include <iostream>
#include "constants.h"

namespace Poco{
    class URI;
    namespace Net{
        class HTTPClientSession;
        class HTTPRequest;
        class HTTPResponse;
    }
}

class ProxyConfiguration;

class Minion{
private:
    int _port_number;
    int size_diff;
    int prev_size;
    std::string _host_name;
    std::string _other_parts;
    Poco::URI *uri;
    Poco::Net::HTTPClientSession *http_session;
    Poco::Net::HTTPRequest *http_request;
    Poco::Net::HTTPResponse *http_response;
    ProxyConfiguration *proxy_config;
    OperationCode op_code;
public:
    Minion();
    ~Minion();
    void start_download_part(std::string, std::string, ProxyConfiguration *pconfig=nullptr);
    long get_size_diff();
};

#endif // MINION


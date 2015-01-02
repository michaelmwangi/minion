#ifndef MINION
#define MINION
#include <iostream>

namespace Poco{
    class URI;
    namespace Net{
        class HTTPClientSession;
        class HTTPRequest;
        class HTTPResponse;
    }
}

class Minion{
private:
    int _startpos;
    std::string _host_name;
    std::string _other_parts;
    Poco::URI *uri;
    Poco::Net::HTTPClientSession *http_session;
    Poco::Net::HTTPRequest *http_request;
    Poco::Net::HTTPResponse *http_response;
public:
    Minion();
    ~Minion();
    void start_download_part(std::string, int);
    std::string print(){
        return "test";
    }
};

#endif // MINION


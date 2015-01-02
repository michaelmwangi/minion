#ifndef DOWNLOADER
#define DOWNLOADER
#include <string>
#include <vector>
#include <thread>

enum class OperationCode{
    None = 0,
    Okay = 200,
    Not_Found = 404,
    Server_Error = 505,
    Bad_Gateway = 502,
    Proxy_Config = 405,
    Resource_Forbidden = 403
};

namespace Poco{
    class URI;
    namespace Net{
        class HTTPRequest;
        class HTTPResponse;
        class HTTPClientSession;
    }
}


class Downloader{
private:
    OperationCode _operation_code;
    Poco::Net::HTTPClientSession *http_session;
    Poco::Net::HTTPRequest *http_request;
    Poco::Net::HTTPResponse *http_response;
    Poco::URI *uri;
    int _num_threads;
    int _num_filechunks;
    int _filesize;
    int _status_code;
    std::string _url;
    std::string _host_name;
    std::string _other_url_parts;
    std::string _filename;
    std::vector<std::thread> minion_workers;
    int calculate_num_threads();
    int calculate_num_filechunks();    
    void clean_up();
    void set_download_metadata();
    void set_operation_code(int);
    void merge_file_parts();
    bool file_part_support();
public:
    Downloader(std::string url);
    ~Downloader();
    std::string get_host();
    std::string get_other_url_parts();
    std::string get_filename();
    OperationCode get_operation_code();
    void set_host(std::string);
    void set_port(int);
    void start_download();    
    void operationcode_error(OperationCode);
    void update_ui();
};

#endif // DOWNLOADER


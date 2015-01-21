#ifndef DOWNLOADER
#define DOWNLOADER
#include <string>
#include <vector>
#include <thread>
#include "constants.h"

namespace Poco{
    class URI;
    namespace Net{
        class HTTPRequest;
        class HTTPResponse;
        class HTTPClientSession;        
    }
}

class ProxyConfiguration;

class Downloader{
private:
    OperationConsts _operation_consts;
    OperationCode _operation_code;
    ProxyConfiguration *proxy_config;
    Poco::Net::HTTPClientSession  *http_session;
    Poco::Net::HTTPRequest *http_request;
    Poco::Net::HTTPResponse *http_response;
    Poco::URI *uri;    
    bool accept_ranges;
    int _num_threads;
    int _num_filechunks;
    int _filesize;
    int _status_code;
    int _port_number;
    std::string _url;
    std::string _host_name;
    std::string _other_url_parts;
    std::string _filename;    
    std::string _home_path;
    std::vector<std::string> file_partnames;
    bool check_accepts_ranges(Poco::Net::HTTPResponse *);
    int _proxy_port;
    int calculate_num_threads();
    int calculate_num_filechunks();    
    void clean_up_resources();
    void set_host_and_otherparts();
    void set_download_metadata();
    void set_operation_code(int);
    void set_filename();
    void merge_file_parts();
    OperationConsts convert_filesize_to_opconsts(int);
    bool file_part_support();  
public:
    Downloader(std::string url, ProxyConfiguration *pconfig=nullptr);
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


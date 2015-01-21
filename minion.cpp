#include "minion.h"
#include "proxyconfiguration.h"
#include "downloaderstats.h"
#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/NullStream.h>
#include <stdlib.h>
#include <istream>
#include <fstream>
#include <thread>
#include <string>
#include <sstream>

using namespace Poco;
using namespace Poco::Net;

Minion::Minion()
    : _port_number(80), _size(0), proxy_config{nullptr}, _done{false}
{}

Minion::~Minion(){
    delete http_request;
    delete http_response;
    delete http_session;
}

void Minion::start_download_part(std::string url, std::string range, ProxyConfiguration *pconfig){
    try{       
        uri = new URI(url);
        _host_name = uri->getHost();
        _other_parts = uri->getPathAndQuery();
        if(_other_parts == "")
            _other_parts = "/";
        http_session = new HTTPClientSession(_host_name, _port_number);
        http_request = new HTTPRequest(HTTPRequest::HTTP_GET, _other_parts, HTTPRequest::HTTP_1_1);
        http_response = new HTTPResponse();
        std::string part = "bytes=" + range;        
        if( pconfig != nullptr){
            proxy_config = pconfig;
            http_request->set("Proxy-Authorization" , "Basic bGVlbGE6bGVlbGExMjM=");
            http_session->setProxy(proxy_config->get_host());
            if( !proxy_config->get_username().empty() ){
                http_session->setProxyCredentials(proxy_config->get_username(), proxy_config->get_password());
                http_session->setProxyPort(proxy_config->get_port());
            }
        }
        http_request->set("Range", part);
        http_session->sendRequest(*http_request);
        std::stringstream ss;
        ss << std::this_thread::get_id();
        std::string fname;
        ss >> fname;
        std::string filepath = getenv("HOME") + std::string("/minion/") + fname;
        std::ofstream file(filepath);
        if(file){            
            std::istream &is = http_session->receiveResponse(*http_response);
            Poco::CountingInputStream c_input_stream(is);
            DownloaderStats d_stats(c_input_stream);

            //I dont think stats_thread is useful here since d_stats.get_size() returns number of characters directly
            //It reports wrong size if stats_thread is removed if i get time will look into
            std::thread stats_thread(&DownloaderStats::run, &d_stats);
            std::thread update_stats_thread([&](){
               while( !d_stats.check_if_done() ){
                    _size = d_stats.get_size();
               }
            });

            StreamCopier::copyStream(c_input_stream, file);
            d_stats.done();
            _done = true;
            stats_thread.join();
            update_stats_thread.join();
            file.close();
        }
        else{
            std::cout<<"Could not open file maybe check permissions?"<<std::endl;
        }
    }
    catch (const Poco::Exception &exc){
        std::cout<<exc.displayText()<<std::endl;
    }
}

long Minion::get_size(){
    return _size;
}

bool Minion::check_if_done(){
    return _done;
}

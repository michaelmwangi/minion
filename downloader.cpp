#include "downloader.h"
#include "downloaderstats.h"
#include "minion.h"
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Exception.h>
#include <functional>
#include <algorithm>
#include <vector>
#include <iostream>
#include <istream>
#include <fstream>


using namespace Poco::Net;
using namespace Poco;

template<typename container1, typename container2, typename lambda>
void launch_minions(container1 &cont1, container2 &cont2, lambda  func){
    auto cont1_it = std::begin(cont1);
    for(;cont1_it != std::end(cont1); ++cont1_it)
        func(cont1_it, cont2);
}

Downloader::Downloader(std::string url)
    : _num_threads{1}, _num_filechunks{1}, _filesize{0}, _url{url}, _filename{"test"}
{
    _operation_code = OperationCode::None;
    uri = new Poco::URI(_url);    
    _host_name = uri->getHost();
    _other_url_parts = uri->getPathAndQuery();
    if(_other_url_parts == "")
        _other_url_parts = "/";
    http_session = new HTTPClientSession(_host_name);
    http_request = new HTTPRequest(HTTPRequest::HTTP_HEAD, _other_url_parts, HTTPRequest::HTTP_1_1);
    http_response = new HTTPResponse();
    set_download_metadata();
}
Downloader::~Downloader(){
    delete uri;
    delete http_session;
    delete http_response;
    delete http_request;
    std::for_each(minion_workers.begin(), minion_workers.end(), std::mem_fn(&std::thread::join));
}

void Downloader::set_download_metadata(){
    _operation_code = OperationCode::None;
    try{
        http_session->sendRequest(*http_request);
        http_session->receiveResponse(*http_response);
        _status_code = http_response->getStatus();
        std::cout<<"Status code : "<<_status_code<<std::endl;
        _filesize = http_response->getContentLength();
    }
    catch (const Poco::Exception &exc){
        http_session->reset();
        std::cout<<"Sorry an error occured "<<exc.displayText()<<std::endl;
    }
}

void Downloader::set_operation_code(int statuscode){
    switch(statuscode){
    case 0:
        _operation_code = OperationCode::None;
        break;
    case 200:
        _operation_code = OperationCode::Okay;
        break;
    case 404:
        _operation_code = OperationCode::Not_Found;
        break;
    case 505:
        _operation_code = OperationCode::Server_Error;
        break;
    case 405:
        _operation_code = OperationCode::Proxy_Config;
        break;
    case 403:
        _operation_code = OperationCode::Resource_Forbidden;
        break;
    default:
        std::cout<<"Status code not documented :"<<statuscode<<std::endl;
    }
}

OperationCode Downloader::get_operation_code(){
    return _operation_code;
}

std::string Downloader::get_filename(){
    return _filename;
}

std::string Downloader::get_host(){
    return _host_name;
}

std::string Downloader::get_other_url_parts(){
    return _other_url_parts;
}

void Downloader::start_download(){
    std::vector<Minion> minions(10);
    launch_minions(minions, minion_workers, [&](
               std::vector<Minion>::iterator &m_it, std::vector<std::thread> &mw){
                    mw.push_back(std::thread(&Minion::start_download_part, &(*m_it), _url,0));
                });
}

void Downloader::operationcode_error(OperationCode opcode){
    switch(opcode){
    case OperationCode::Bad_Gateway:
        std::cout<<"Server sent back 502 status code -> bad gateway"<<std::endl;
        break;
    case OperationCode::Server_Error:
        std::cout<<"Server sent back 505 status code -> server error"<<std::endl;
        break;
    case OperationCode::None:
        std::cout<<"Nothing done yet by the minion"<<std::endl;
        break;
    case OperationCode::Not_Found:
        std::cout<<"Server sent back 404 status code -> resource not found"<<std::endl;
        break;
    case OperationCode::Proxy_Config:
        std::cout<<"Server sent back 405 status code -> please configure for proxy"<<std::endl;
        break;
    case OperationCode::Resource_Forbidden:
        std::cout<<"Server sent back 403 status code -> Access to the resource is forbidden"<<std::endl;
        break;
    default:
        //we should never get here
        std::cout<<"Unknown operational code"<<std::endl;
    }
}

int Downloader::calculate_num_filechunks(){
    //this is just dumb
    _num_filechunks = 10;
    return _num_filechunks;
}

int Downloader::calculate_num_threads(){
    //this is also just dumb
    _num_threads = 10;
    return _num_threads;
}

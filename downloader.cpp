#include "downloader.h"
#include "downloaderstats.h"
#include "minion.h"
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Exception.h>
#include <Poco/File.h>
#include <stdlib.h>
#include <mutex>
#include <algorithm>
#include <vector>
#include <iostream>
#include <istream>
#include <fstream>
#include <sstream>

using namespace Poco::Net;
using namespace Poco;

//this function should be moved inside the Downloader class -> To Do
template<typename container1, typename container2, typename lambda>
void launch_minions(container1 &cont1, container2 &cont2, lambda  func){
    auto cont1_it = std::begin(cont1);
    for(;cont1_it != std::end(cont1); ++cont1_it)
        func(cont1_it, cont2);
}

Downloader::Downloader(std::string url, std::string proxyhost, std::string proxyusername, std::string proxypassword, int proxyport)
    :accept_ranges{false}, _num_threads{1}, _num_filechunks{1}, _filesize{0}, _url{url}, _filename{""}, _port_number{80},
    _proxy_host{proxyhost}, _proxy_username{proxyusername}, _proxy_password{proxypassword}, _proxy_port{proxyport}
{
    _operation_code = OperationCode::None;
    uri = new Poco::URI(_url);    
    _host_name = uri->getHost();
    _other_url_parts = uri->getPathAndQuery();
    if(_other_url_parts == "")
        _other_url_parts = "/";
    http_session = new HTTPClientSession(_host_name, _port_number);
    http_request = new HTTPRequest(HTTPRequest::HTTP_HEAD, _other_url_parts, HTTPRequest::HTTP_1_1);
    http_response = new HTTPResponse();
    if(!_proxy_host.empty()){
        http_request->set("Proxy-Authorization" , "Basic bGVlbGE6bGVlbGExMjM=");
        //set proxy port also here
        http_session->setProxy(_proxy_host);
        if(!proxyusername.empty()){
            http_session->setProxyCredentials(_proxy_username, _proxy_password);
        }
    }
    set_download_metadata();
    //create the minion directory partial files are stored here
    _home_path = getenv("HOME") + std::string("/minion");
    File file(_home_path);
    file.createDirectory();
}

Downloader::~Downloader(){
    delete uri;
    delete http_session;
    delete http_response;
    delete http_request;
}

bool Downloader::check_accepts_ranges(HTTPResponse *response){
    try{
        response->get("Accept-Ranges");
        return true;
    }
    catch(const Poco::Exception &exc){
        return false;
    }
}

void Downloader::merge_file_parts(){
    //TO Do check status of the process here i.e if everything went ok
    std::ofstream file(_filename);

    if(file.is_open()){
        for(auto part_name : file_partnames){
            std::string filepath = _home_path + "/" + part_name;
            std::ifstream ifile(filepath);
            if(ifile.is_open()){
                    StreamCopier::copyStream(ifile, file);
                    ifile.close();
            }
        }
        file.close();
    }
    else{
        std::cout<<"error creating file maybe permissions ?"<<std::endl;
    }
}

void Downloader::set_filename(){
    if(_other_url_parts != ""){
        std::vector<char> vec;
        for(auto it = _url.rbegin(); it != _url.rend() ;++it){
            if(*it == '/')
                break;
            vec.push_back(*it);
        }
        //vec is 'fed' in opposite to reverse the string back correctly
        std::string temp = std::string(vec.rbegin(), vec.rend());
        uri->decode(temp, _filename);

    }
    else{
            _filename = _host_name;
        }
}

void Downloader::set_download_metadata(){
    _operation_code = OperationCode::None;
    try{
        http_session->sendRequest(*http_request);
        http_session->receiveResponse(*http_response);
        _status_code = http_response->getStatus();
        std::cout<<"Status code : "<<_status_code<<std::endl;
        std::cout<<"host : "<< _host_name<<std::endl;
        _filesize = http_response->getContentLength();        
        std::cout<<"filesize is  "<<_filesize<<" bytes"<<std::endl;
        accept_ranges = check_accepts_ranges(http_response) == true ? true : false;
        set_filename();
    }
    catch (const Poco::Exception &exc){
        std::cout<<"error code  ->"<< exc.name()<<std::endl;
        std::cout<<"Sorry an error occured "<<exc.displayText()<<std::endl;
        http_session->reset();
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
    int file_chunks = 1;
    int chunk_size = _filesize;
    if(accept_ranges == true ){
        file_chunks = calculate_num_filechunks();
        chunk_size = _filesize / file_chunks;
    }
    std::vector<Minion> minions(file_chunks);
    std::vector<std::thread> minion_workers;
    std::vector<std::string> ranges;
    for(int i=0;i<file_chunks;++i){
        //variables initialized below
        std::string range;
        int  current_sz;
        if(i == 0){
            range = std::to_string(0) + "-" + std::to_string(chunk_size);
            current_sz = chunk_size;
        }
        else if(i == (file_chunks - 1)){
            range  = std::to_string(current_sz) + "-";
        }
        else{
            range = std::to_string(current_sz) + "-" + std::to_string(current_sz + chunk_size);
            current_sz += chunk_size;
        }
        ranges.push_back(range);
    }
    launch_minions(minions, minion_workers,
                   [&](std::vector<Minion>::iterator &m_it, std::vector<std::thread> &mw){
                            std::string range = ranges.at(mw.size());
                            mw.push_back(std::thread(&Minion::start_download_part, &(*m_it), _url, range));
                });
    for(auto &worker_thread : minion_workers){
        std::stringstream ss;
        ss << worker_thread.get_id();
        std::string name;
        ss >> name;
        file_partnames.push_back(name);
        worker_thread.join();
    }
    merge_file_parts();
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

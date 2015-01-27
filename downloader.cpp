#include "downloader.h"
#include "proxyconfiguration.h"
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
#include <cstdio>
#include <cstdlib> //exit  EXIT_FAILURE
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

void Downloader::set_host_otherparts_port(){
    _host_name = uri->getHost();
    _other_url_parts = uri->getPathAndQuery();
    if(_other_url_parts == "")
        _other_url_parts = "/";
    auto index = _url.find_first_of(":");
    //check if we have the protocol
    // length("//") == 2
    if( !(_url.substr(index, 3) ==  "://") ){
        _url = "http://" + _url;
    }
    auto index2 = _url.find_last_of(":");
    if( index2 != std::string::npos  && index2 != index){
        auto port_len = 0;
        auto init = index2;
        while(_url.at(init) != '/'){
            ++init;
            ++port_len;
        }
        auto port_str = _url.substr(index2, port_len);
        auto  port = std::stoi(port_str);
        uri->setPort(port);
    }
    else{
        //default port ->80
        uri->setPort(80);
    }
}

Downloader::Downloader(std::string url, ProxyConfiguration *pconfig)
    :proxy_config{pconfig}, accept_ranges{false}, _num_threads{1}, _num_filechunks{1}, _filesize{0}, _url{url}, _filename{""}, _port_number{80}
{
    _operation_code = OperationCode::None;
    //check if the passsed url has protocol if not append to it
    //matches the first instance of ':' which should belong to the protocol,yeah i know a more robust way of getting the right protocol is needed
    uri = new Poco::URI(_url);    
    set_host_otherparts_port();
    http_session = new HTTPClientSession(_host_name, _port_number);
    http_request = new HTTPRequest(HTTPRequest::HTTP_HEAD, _other_url_parts, HTTPRequest::HTTP_1_1);
    http_response = new HTTPResponse();
    if(proxy_config != nullptr){
        http_request->set("Proxy-Authorization" , "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==");
        http_session->setProxy(proxy_config->get_host());
        if( !proxy_config->get_username().empty() ){
            http_session->setProxyCredentials(proxy_config->get_username(), proxy_config->get_password());
            http_session->setProxyPort(proxy_config->get_port());
        }
    }
    set_download_metadata();
    //create the minion directory partial files are stored here
    _home_path = getenv("HOME") + std::string("/minion");
    File file(_home_path);
    file.createDirectory();
}



Downloader::~Downloader(){
    clean_up_resources();
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
    std::ofstream file(_filename, std::ios::binary);

    if(file.is_open()){
        for(auto part_name : file_partnames){
            std::string filepath = _home_path + "/" + part_name;
            std::ifstream ifile(filepath, std::ios::binary);
            if(ifile.is_open()){
                    StreamCopier::copyStream(ifile, file);
                    ifile.close();
            }
            //clean up the file part
            std::remove(filepath.c_str());
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

void Downloader::clean_up_resources(){
    delete uri;
    delete http_request;
    delete http_response;
    delete http_session;
}

void Downloader::set_download_metadata(){
    _operation_code = OperationCode::None;
    //http_request->set("User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:12.0) Gecko/20100101 Firefox/21.0");
    http_session->sendRequest(*http_request);
    http_session->receiveResponse(*http_response);
    _status_code = http_response->getStatus();
    set_operation_code(_status_code);

    if(get_operation_code() == OperationCode::Moved){
        std::cout<<"moved"<<std::endl;
        //resource moved, get the url and start new connection
        _url = http_response->get("Location");
        http_session->reset();
        clean_up_resources();
        uri = new URI(_url);
        set_host_otherparts_port();
        http_request = new HTTPRequest(HTTPRequest::HTTP_GET, _other_url_parts, HTTPRequest::HTTP_1_1);
        http_session = new HTTPClientSession(_host_name, _port_number);
        http_response = new HTTPResponse();
        http_session->sendRequest(*http_request);
        http_session->receiveResponse(*http_response);
    }
    else if(get_operation_code() != OperationCode::Okay){
        operationcode_error(get_operation_code());
        clean_up_resources();
        exit(EXIT_FAILURE);
    }
    _filesize = http_response->getContentLength();
    if( _filesize < 0 ){
        //initiate a mock request since server did not send content length with http:head request
        http_session->reset();
        delete http_request;
        http_request = new HTTPRequest(HTTPRequest::HTTP_GET, _other_url_parts, HTTPRequest::HTTP_1_1);
        http_session->sendRequest(*http_request);
        http_session->receiveResponse(*http_response);
        _filesize = http_response->getContentLength();
        http_session->reset();
    }
    if (_filesize > 0){
        std::cout<<"filesize is  "<<_filesize<<" bytes"<<std::endl;
    }
    else{
        std::cout<<"filesize is  "<<"~"<<" bytes"<<std::endl;
    }
    accept_ranges = check_accepts_ranges(http_response) == true ? true : false;
    set_filename();
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
    case 407:
        _operation_code = OperationCode::Proxy_Config;
        break;
    case 403:
        _operation_code = OperationCode::Resource_Forbidden;
        break;
    case 302:
        _operation_code = OperationCode::Moved;
        break;
    case 503:
        _operation_code = OperationCode::Service_Unavailable;
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
    std::vector<Minion> minions(file_chunks);
    std::vector<std::thread> minion_workers;
    std::vector<std::string> ranges;
    if(accept_ranges == true && _filesize > 0){
        file_chunks = calculate_num_filechunks();
        chunk_size = _filesize / file_chunks;
        for(int i=0;i<file_chunks;++i){
            //variables initialized below
            std::string range;
            int  current_sz;
            if(i == 0){
                range = std::to_string(0) + "-" + std::to_string(chunk_size);
                current_sz = chunk_size + 1;
            }
            else if(i == (file_chunks - 1)){
                range  = std::to_string(current_sz) + "-";
            }
            else{
                range = std::to_string(current_sz) + "-" + std::to_string(current_sz + chunk_size);
                current_sz += (chunk_size+1);
            }
            ranges.push_back(range);
        }
    }
    else{
        std::string range = "0-";
        ranges.push_back(range);
    }
    launch_minions(minions, minion_workers,
                   [&](std::vector<Minion>::iterator &m_it, std::vector<std::thread> &mw){
                            std::string range = ranges.at(mw.size());
                            mw.push_back(std::thread(&Minion::start_download_part, &(*m_it), _url, range, proxy_config));
                });
//    std::thread update_stats_thread([&](){
//       bool finished = false;
//       while( !finished){
//           bool minion_completed = true;
//           auto size = 0L;
//           for(auto &minion : minions){
//               if( !minion.check_if_done() ){
//                   minion_completed = false;
//               }
//               size += minion.get_size();
//           }
//           auto percent_progress = ( ((float)size/(float)_filesize)) *100 ;
//           std::cout<<size<<"\r"<<"\t"<<percent_progress<<" %"<<"\r";
//           if(minion_completed){
//               std::cout<<std::endl;
//               finished = true;
//               std::cout<<"completed"<<std::endl;
//           }
//       }
//    });
    std::cout<<"download in progress please wait"<<std::endl;
    for(auto &worker_thread : minion_workers){
        std::stringstream ss;
        ss << worker_thread.get_id();
        std::string name;
        ss >> name;
        file_partnames.push_back(name);
        worker_thread.join();
    }
//    update_stats_thread.join();
    std::cout<<"completed"<<std::endl;
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
    case OperationCode::Service_Unavailable:
        std::cout<<"Sorry but the service seems unavailable"<<std::endl;
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

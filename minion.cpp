#include "minion.h"
#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <stdlib.h>
#include <istream>
#include <fstream>
#include <thread>
#include <string>
#include <sstream>

using namespace Poco;
using namespace Poco::Net;

Minion::Minion()
    : _port_number(80)
{}

Minion::~Minion(){
    delete http_request;
    delete http_response;
    delete http_session;
}

void Minion::start_download_part(std::string url, std::string range){
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
        std::cout<<"my range is "<<part<<std::endl;
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
            StreamCopier::copyStream(is, file);
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

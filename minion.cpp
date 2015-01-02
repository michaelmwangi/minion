#include "minion.h"
#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <istream>
#include <fstream>
#include <thread>
#include <string>

using namespace Poco;
using namespace Poco::Net;

Minion::Minion()
    : _startpos(0)
{}

Minion::~Minion(){
    delete http_request;
    delete http_response;
    delete http_session;
}

void Minion::start_download_part(std::string url, int startpos){
    try{
        _startpos = startpos;
        uri = new URI(url);
        _host_name = uri->getHost();
        _other_parts = uri->getPathAndQuery();
        if(_other_parts == "")
            _other_parts = "/";
        http_session = new HTTPClientSession(_host_name);
        http_request = new HTTPRequest(HTTPRequest::HTTP_HEAD, _other_parts, HTTPRequest::HTTP_1_1);
        http_response = new HTTPResponse();
        std::string part = "bytes=" + std::to_string(_startpos);
        http_request->set("Range", part);
        http_session->sendRequest(*http_request);
        std::string filename = "test.txt";//std::to_string(std::this_thread::get_id().id());
        std::ofstream file(filename);
        if(file){
            std::istream &is = http_session->receiveResponse(*http_response);
            StreamCopier::copyStream(is, file);
            file.close();
        }
        else{
            std::cout<<"Could not open file"<<std::endl;
        }
    }
    catch (const Poco::Exception &exc){
        std::cout<<exc.displayText()<<std::endl;
    }
}

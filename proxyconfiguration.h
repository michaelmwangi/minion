#ifndef PROXYCONFIGURATION
#define PROXYCONFIGURATION
#include <string>
#include <algorithm>
#include <cstddef>
#include <Poco/URI.h>

class ProxyConfiguration{
private:
    std::string _host;
    std::string _username;
    std::string _password;
    int _port;
public:
    ProxyConfiguration(std::string host, std::string username, std::string password, int port)
        :_host{host}, _username{username}, _password{password}, _port{port}
    {
    }
    ProxyConfiguration(std::string proxystringcompact){
        //assuming Http proxy
        std::string protocol = "http://";
        std::string noprotocol = proxystringcompact.substr(protocol.length());
        auto it = std::find_if(noprotocol.begin(), noprotocol.end(), [](const char &let){
            return (let == ':');
        });

        if(it != noprotocol.end()){
            //extracting the username
            auto start_it = noprotocol.begin();
            auto length = std::distance(start_it, it);
            auto username = noprotocol.substr(0, length);
            //extracting the password
            auto pass_it = std::find_if(it, noprotocol.end(), [](const char &let){
                return (let == '@');
            });
            length = std::distance(start_it+username.length(), pass_it);
            auto password = noprotocol.substr(username.length()+1, length-1);
            //extract the host
            auto host_it = std::find_if(pass_it, noprotocol.end(),[](const char &let){
               return (let == ':');
            });
            length = std::distance(pass_it, host_it);
            auto start_pos = username.length()+1 + password.length()+1;
            auto host = noprotocol.substr(start_pos, length-1);
            //extract the port number
            start_pos += host.length()+1;
            length = std::distance(host_it, noprotocol.end());
            auto port = noprotocol.substr(start_pos, length-1);

            Poco::URI uri;
            uri.decode(username, _username);
            uri.decode(password, _password);
            _host =  host;
            _port = std::stoi(port);
        }

    }
    int get_port(){
        return _port;
    }
    std::string get_host(){
        return _host;
    }
    std::string get_username(){
        return _username;
    }
    std::string get_password(){
        return _password;
    }
};

#endif // PROXYCONFIGURATION


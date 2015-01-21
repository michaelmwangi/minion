#include <iostream>
#include <stdlib.h>
#include "downloader.h"
#include "optionparser.h"
#include "proxyconfiguration.h"

struct Arg: public option::Arg{
    static void print_error(std::string msg){
        std::cout<<msg<<std::endl;
    }
    static option::ArgStatus Required(const option::Option& option, bool msg){
        if(option.arg != 0){
            return option::ARG_OK;
        }
        if(msg){
            std::string message = "An argument is required";
            print_error(message);
        }
        return option::ARG_ILLEGAL;
    }
};

enum  OptionIndex {HELP,URL,PROXY,UNKNOWN};

const option::Descriptor usage[]= {
  {OptionIndex::HELP, 0, "h", "help", Arg::None, "--help, -h print this help and exit."},
  {OptionIndex::PROXY, 0, "p", "proxy",Arg::Required, "--proxy, -p set http proxy."},
  {OptionIndex::URL, 0, "u", "url", Arg::Required, "--url, -u set the download url."},
  {OptionIndex::UNKNOWN, 0, "", "", Arg::None, "Unkown args passed see --help for usage"}
};

int main(int argc, char *argv[])
{
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    option::Stats  stats(usage, argc, argv);
    option::Option options[4];
    option::Option buffer[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);
    if(parse.error()){
        std::cout<<"sorry encountered error in parsing the args see --help"<<std::endl;
        return 1;
    }
    //check for help option or no option
//    if(options[OptionIndex::HELP] || argc == 0){
//        std::cout<<"arguments -h --help  print this help and exit"<<std::endl;
//        std::cout<<"          -p --proxy <proxy>  set proxy"<<std::endl;
//        std::cout<<"          -u --url <url> set the url"<<std::endl;
//        return 0;
//    }
    ProxyConfiguration *p_config=nullptr;
    std::string url = std::string();
    for(int i = 0;i< parse.optionsCount();++i){
        option::Option &opt = buffer[i];
        switch(opt.index()){
        case OptionIndex::PROXY:
            p_config = new ProxyConfiguration(opt.arg);
            break;
        case OptionIndex::URL:
            url = opt.arg;
            break;
        case OptionIndex::UNKNOWN:
            std::cout<<"unkown arguments passed"<<std::endl;
            break;
        }
    }
    url = "www.facebook.com";
    if( url.empty()){
        std::cout<<"Url argument not passed cannot continue"<<std::endl;
        return 0;
    }

    Downloader down(url, p_config);
    down.start_download();
    return 0;
}


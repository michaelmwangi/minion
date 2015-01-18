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
  {OptionIndex::URL, 1, "u", "url", Arg::Required, "--url, -u set the download url."},
  {OptionIndex::UNKNOWN, 0, "", "", Arg::None, "Unkown args passed see --help for usage"}
};

int main(int argc, char *argv[])
{    
   ProxyConfiguration p_config("http://I08%2F1106%2F2011%40students:t34%40uon@proxy.uonbi.ac.ke:80");
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
    if(options[OptionIndex::HELP] || argc == 0){
        //int columns = getenv("COLUMNS")? atoi(getenv("COLUMNS")) : 80;
        std::cout<<"Help message"<<std::endl;//option::printUsage(fwrite, stdout, usage, columns);
        return 0;
    }
    for(int i = 0;i< parse.optionsCount();++i){
        option::Option &opt = buffer[i];
        switch(opt.index()){
        case OptionIndex::PROXY:
            std::cout<<"settin proxy here "<<opt.arg<<"done"<<std::endl;
            break;
        case OptionIndex::URL:
            std::cout<<"settin the url here "<<opt.arg<<std::endl;
            break;
        case OptionIndex::UNKNOWN:
            std::cout<<"unkown arguments passed"<<std::endl;
            break;
        }
    }
    /*
    std::string proxyhost = "proxy.uonbi.ac.ke";
    std::string proxyusername = "I08/1106/2011@students";
    std::string proxypassword = "t34@uon";
    std::string url = "http://127.0.0.1/download/Brick.Mansions.2014.720p.BluRay.x264.YIFY.mp4";
    Downloader down(url);

    down.start_download();*/
    return 0;
}


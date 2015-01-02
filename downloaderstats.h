#ifndef DOWNLOADERSTATS
#define DOWNLOADERSTATS
#include <Poco/CountingStream.h>
#include <iostream>

using namespace Poco;

class DownloaderStats{
private:
    const CountingInputStream &_is;
    bool _done;
public:
    DownloaderStats(const CountingInputStream &is)
        :_is(is), _done(false)
    {}
    void run(){
        while(!_done){
            std::cout<<_is.chars()<<std::endl;
        }
    }
    void done(){
        _done = true;
    }
};
#endif // DOWNLOADERSTATS


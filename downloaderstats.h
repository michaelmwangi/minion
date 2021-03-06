#ifndef DOWNLOADERSTATS
#define DOWNLOADERSTATS
#include <Poco/CountingStream.h>
#include <iostream>

using namespace Poco;

class DownloaderStats{
private:
    CountingInputStream &_is;
    bool _done;
    long _size;
public:
    DownloaderStats(CountingInputStream &is)
        :_is(is), _done(false), _size(0)
    {}
    void run(){
        while(!_done)
            _size = _is.chars();
    }
    long get_size(){
        return _is.chars();
    }
    void done(){
        _done = true;
    }
    bool check_if_done(){
        return _done;
    }
};
#endif // DOWNLOADERSTATS


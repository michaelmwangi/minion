#include <iostream>
#include <stdlib.h>
#include "downloader.h"
#include <algorithm>

using namespace std;

int main()
{
    std::string proxyhost = "proxy.uonbi.ac.ke";
    std::string proxyusername = "I08/1106/2011@students";
    std::string proxypassword = "t34@uon";
    std::string url = "http://av.vimeo.com/59831/449/6952471.flv?download=1&token2=1421399598_245dc80c4c25cd7009c450ee1b553008&filename=Sia%2520%7C%2520Buttons%2520%28Music%2520Video%29-HD.flv";
    Downloader down(url, proxyhost,proxyusername,proxypassword);

    down.start_download();
    return 0;
}


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
    Downloader down("http://srv55.vidtomp3.com/download/4pSWaG9mnK6npa9tm9+UbGthnGJmZGxu4rnMnK2f1aJn/Slipknot%20-%20Vermillion%20Pt.%202%20%5BOFFICIAL%20VIDEO%5D.mp3", proxyhost,proxyusername,proxypassword);

    down.start_download();
    return 0;
}


#include <iostream>
#include "downloader.h"

using namespace std;

int main()
{
    Downloader down("http://127.0.0.1");
    cout << "Hello World!" << endl;
    down.start_download();
    return 0;
}


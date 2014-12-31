#include <iostream>
#include "downloader.h"

using namespace std;

int main()
{
    Downloader down("http://127.0.0.1");
    down.start_download();
    cout << "Hello World!" << endl;
    return 0;
}


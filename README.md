# minion
simple stupid multithreaded downloader
To compile make sure you have installed Poco c++  libraries and issue

g++ -o minion  main.cpp downloader.cpp minion.cpp  -std=c++11 -lPocoNet -lPocoFoundation


#What works

1. Works with http proxy
2. For filesizes larger than 1Mb it is split to 10 parts which are downloaded in parallel and thereafter merged into a single file
3. I have yet to come up with a better algorithm for splitting the files

#To do

1. Add download progress data support 
2. Add better messaging system
3. Add config files support
4. Add resume download support
5. Add youtube download support

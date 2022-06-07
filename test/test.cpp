#include <parse.hpp>
#include <request.hpp>

#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

int main(){
    std::locale::global(std::locale(""));
    std::string xml=request("http://speedboatdope.com/rss/");
    if(xml==""){
        std::cout <<" not found"<< std::endl;
    }
    else{
        std::string last=parsepodcast(xml, "629 - Night at the Opera feat. Tim Heidecker (5/19/22)");
        std::cout << last << std::endl;
    }
    return 0;
}

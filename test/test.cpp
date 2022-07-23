#include <request.hpp>
#include <parse.hpp>

#include <iostream>
#include <string>
#include <chrono>
#include <cassert>
#include <thread>
#include <filesystem>

int main(){
    std::locale::global(std::locale(""));
    //test output
    std::string xml=request("https://www.youtube.com/");
    assert(!xml.empty());
    //test redirect limit
    xml=request("https://www.webtoons.com");
    assert(xml=="");
    //check only one initialization
    for(int i=0;i<3;i++){
        static Python py;
        py.download("resto",{});
    }
    return 0;
}

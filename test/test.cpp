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
    //test no exception in constructor
    Youtube_Parse parser;
    //test output
    std::string xml=request("https://www.youtube.com/");
    assert(!xml.empty());
    //test redirect limit
    xml=request("https://www.webtoons.com");
    assert(xml=="");
    return 0;
}

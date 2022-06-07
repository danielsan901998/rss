#include <request.hpp>

#include <iostream>
#include <string>
#include <chrono>
#include <cassert>
#include <thread>

int main(){
    std::locale::global(std::locale(""));
    //test redirect limit
    std::string xml=request("https://www.webtoons.com");
    assert(xml=="");
    return 0;
}

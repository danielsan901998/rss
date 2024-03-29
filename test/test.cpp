#include <request.hpp>
#include <parse.hpp>

#include <iostream>
#include <string>
#include <chrono>
#include <cassert>
#include <thread>
#include <filesystem>

int main(){
    //test output
    std::string xml=request("https://www.youtube.com/");
    assert(!xml.empty());
    //check only one initialization
    for(int i=0;i<3;i++){
        static Python py;
        py.download("resto",{"https://www.youtube.com/shorts/amz0mvUmVoc","https://www.youtube.com/shorts/avCzmwL0Krg"});
    }
    return 0;
}

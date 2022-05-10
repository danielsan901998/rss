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
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{}};

    auto col = conn["database"]["blogs"];
    auto cursor = col.find({});
    for (const auto& doc : cursor) {
        std::string blog= doc["blog"].get_utf8().value.to_string();
        std::string articulo= doc["articulo"].get_utf8().value.to_string();
        std::string last;
        std::string xml=request(doc["url"].get_utf8().value.to_string());
        if(xml==""){
            std::cout << blog << " not found"<< std::endl;
        }
        else{
            if(doc["contain"]) {
                last=parseblog(xml, articulo, doc["contain"].get_utf8().value.to_string());
            }
            else {
                last=parseblog(xml, articulo, "");
            }
        }
    }
    for (const auto& doc : cursor) {
        std::string podcast= doc["nombre"].get_utf8().value.to_string();
        std::string articulo= doc["ultimo"].get_utf8().value.to_string();
        std::string xml=request(doc["url"].get_utf8().value.to_string());
        if(xml==""){
            std::cout << podcast << " not found"<< std::endl;
        }
        else{
            std::string last=parsepodcast(xml, articulo);
        }
    }
    return 0;
}

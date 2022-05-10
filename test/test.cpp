#include <parse.hpp>
#include <request.hpp>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

namespace fs = std::filesystem;

int main(){
    std::locale::global(std::locale(""));
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{}};
    std::string folder="xml/";
    fs::create_directories(folder);

    auto col = conn["database"]["blogs"];
    auto cursor = col.find({});
    for (const auto& doc : cursor) {
        std::string blog= doc["blog"].get_utf8().value.to_string();
        std::string articulo= doc["articulo"].get_utf8().value.to_string();
        std::string last;
        std::string xml;
        if(!fs::exists(folder+blog)){
            xml=request(doc["url"].get_utf8().value.to_string());
            if(xml==""){
                std::cout << blog << " not found"<< std::endl;
            }
            else{
                std::ofstream out(folder+blog);
                out << xml;
            }
        }else{
            std::ifstream t(folder+blog);
            std::stringstream buffer;
            buffer << t.rdbuf();
            xml=buffer.str();
        }
        if(doc["contain"]) {
            last=parseblog(xml, articulo, doc["contain"].get_utf8().value.to_string());
        }
        else {
            last=parseblog(xml, articulo, "");
        }
        std::cout << blog << ": " << last << "\n---------"<< std::endl;
    }

    col = conn["database"]["podcast"];
    cursor = col.find({});
    for (const auto& doc : cursor) {
        std::string blog= doc["nombre"].get_utf8().value.to_string();
        std::string articulo= doc["ultimo"].get_utf8().value.to_string();
        std::string xml;
        if(!fs::exists(folder+blog)){
            xml=request(doc["url"].get_utf8().value.to_string());
            if(xml==""){
                std::cout << blog << " not found"<< std::endl;
            }
            else{
                std::ofstream out(folder+blog);
                out << xml;
            }
        }else{
            std::ifstream t(folder+blog);
            std::stringstream buffer;
            buffer << t.rdbuf();
            xml=buffer.str();
        }
        std::string last=parsepodcast(xml, articulo);
        std::cout << blog << ": " << last << "\n---------"<< std::endl;
    }

    col = conn["database"]["youtube"];
    bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result = col.find_one(
            bsoncxx::builder::stream::document{}<<"hora"<<bsoncxx::builder::stream::open_document <<"$exists"<<true<<bsoncxx::builder::stream::close_document <<bsoncxx::builder::stream::finalize);
    if(maybe_result){
        bsoncxx::document::element hora =  maybe_result.value().view()["hora"];
        std::time_t date = std::chrono::system_clock::to_time_t(hora.get_date());
        cursor = col.find(bsoncxx::builder::stream::document{}<<"descargar"<<true<<bsoncxx::builder::stream::finalize);
        for (const auto& doc : cursor) {
            std::string blog=doc["nombre"].get_utf8().value.to_string();
            std::string xml;
            if(!fs::exists(folder+blog)){
                xml=request("https://www.youtube.com/feeds/videos.xml?channel_id="+doc["id"].get_utf8().value.to_string());
                if(xml==""){
                    std::cout << blog << " not found"<< std::endl;
                }
                else{
                    std::ofstream out(folder+blog);
                    out << xml;
                }
            }else{
                std::ifstream t(folder+blog);
                std::stringstream buffer;
                buffer << t.rdbuf();
                xml=buffer.str();
            }
            std::time_t time=parseyoutube(xml,date,doc);
            std::cout << blog << ": " << time << std::endl;
            return 0;
        }
    }
    return 0;
}

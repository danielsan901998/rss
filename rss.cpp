#include "rss.hpp"

int main(int argc, char* argv[]){
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
            if(!last.empty() && last!=articulo){
                col.update_one(
                        bsoncxx::builder::stream::document{} << "blog" << blog << bsoncxx::builder::stream::finalize,  
                        bsoncxx::builder::stream::document{} << "$set" << bsoncxx::builder::stream::open_document << "articulo" << last << bsoncxx::builder::stream::close_document <<bsoncxx::builder::stream::finalize);  
            }
        }
    }

    col = conn["database"]["podcast"];
    cursor = col.find({});
    for (const auto& doc : cursor) {
        std::string podcast= doc["nombre"].get_utf8().value.to_string();
        std::string articulo= doc["ultimo"].get_utf8().value.to_string();
        std::string xml=request(doc["url"].get_utf8().value.to_string());
        if(xml==""){
            std::cout << podcast << " not found"<< std::endl;
        }
        else{
            std::string last=parsepodcast(xml, articulo);
            if(!last.empty() && last!=articulo){
                col.update_one(
                        bsoncxx::builder::stream::document{} << "nombre" << podcast << bsoncxx::builder::stream::finalize,
                        bsoncxx::builder::stream::document{} << "$set"<< bsoncxx::builder::stream::open_document << "ultimo" << last << bsoncxx::builder::stream::close_document <<bsoncxx::builder::stream::finalize);
            }
        }
    }

    col = conn["database"]["youtube"];
    bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result = col.find_one(
            bsoncxx::builder::stream::document{}<<"hora"<<bsoncxx::builder::stream::open_document <<"$exists"<<true<<bsoncxx::builder::stream::close_document <<bsoncxx::builder::stream::finalize);
    if(maybe_result){
        bsoncxx::document::element hora =  maybe_result.value().view()["hora"];
        std::time_t date = std::chrono::system_clock::to_time_t(hora.get_date());
        std::time_t last=date;
        cursor = col.find(bsoncxx::builder::stream::document{}<<"descargar"<<true<<bsoncxx::builder::stream::finalize);
        for (const auto& doc : cursor) {
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            std::string nombre=doc["nombre"].get_utf8().value.to_string();
            std::string xml=request("https://www.youtube.com/feeds/videos.xml?channel_id="+doc["id"].get_utf8().value.to_string());
            if(xml==""){
                std::cout << nombre << " not found"<< std::endl;
            }
            else{
                std::time_t time=parseyoutube(xml,date,doc);
                if(time>last)last=time;
            }
        }
        if(last!=date){
            bsoncxx::types::b_date doc=bsoncxx::types::b_date{
                std::chrono::system_clock::from_time_t(last)
            };
            col.update_one(
                    bsoncxx::builder::stream::document{}<<"hora"<<bsoncxx::builder::stream::open_document <<"$exists"<<true<<bsoncxx::builder::stream::close_document <<bsoncxx::builder::stream::finalize,
                    bsoncxx::builder::stream::document{} << "$set"<< bsoncxx::builder::stream::open_document << "hora" << doc << bsoncxx::builder::stream::close_document <<bsoncxx::builder::stream::finalize);
        }
    }
    return 0;
}

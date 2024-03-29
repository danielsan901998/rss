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
	mongocxx::instance inst{};
	mongocxx::client conn{mongocxx::uri{}};

	auto col = conn["database"]["blogs"];
	auto cursor = col.find({});
	for (const auto& doc : cursor) {
		std::string blog= doc["blog"].get_utf8().value.to_string();
		std::string articulo= doc["articulo"].get_utf8().value.to_string();
		std::string last;
		std::string xml=request(doc["url"].get_utf8().value.to_string());
		if(xml.empty()){
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
			std::this_thread::sleep_for(std::chrono::milliseconds(400));
		}
	}

	col = conn["database"]["podcast"];
	cursor = col.find({});
	for (const auto& doc : cursor) {
		std::string podcast= doc["nombre"].get_utf8().value.to_string();
		std::string articulo= doc["ultimo"].get_utf8().value.to_string();
		std::string xml=request(doc["url"].get_utf8().value.to_string());
		if(xml.empty()){
			std::cout << podcast << " not found"<< std::endl;
		}
		else{
			std::string last=parsepodcast(xml, articulo);
			if(!last.empty() && last!=articulo){
				col.update_one(
						bsoncxx::builder::stream::document{} << "nombre" << podcast << bsoncxx::builder::stream::finalize,
						bsoncxx::builder::stream::document{} << "$set"<< bsoncxx::builder::stream::open_document << "ultimo" << last << bsoncxx::builder::stream::close_document <<bsoncxx::builder::stream::finalize);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(400));
		}
	}

	col = conn["database"]["youtube"];
	bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result = col.find_one(
			bsoncxx::builder::stream::document{}<<"hora"<<bsoncxx::builder::stream::open_document <<"$exists"<<true<<bsoncxx::builder::stream::close_document <<bsoncxx::builder::stream::finalize);
	if(maybe_result){
		try{
			bsoncxx::document::element hora =  maybe_result.value().view()["hora"];
			std::time_t date = std::chrono::system_clock::to_time_t(hora.get_date());
			std::time_t last=date;
			cursor = col.find(bsoncxx::builder::stream::document{}<<"descargar"<<true<<bsoncxx::builder::stream::finalize);
			for (const auto& doc : cursor) {
				std::string nombre=doc["nombre"].get_utf8().value.to_string();
				std::string xml=request("https://rsshub.app/youtube/channel/"+doc["id"].get_utf8().value.to_string());
				if(xml.empty()){
					std::cout << nombre << " not found"<< std::endl;
				}
				else{
					std::time_t time=parseyoutube(xml,date,doc);
					if(time>last)last=time;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(400));
			}
			if(last!=date){
				bsoncxx::types::b_date doc=bsoncxx::types::b_date{
					std::chrono::system_clock::from_time_t(last)
				};
				col.update_one(
						bsoncxx::builder::stream::document{}<<"hora"<<bsoncxx::builder::stream::open_document <<"$exists"<<true<<bsoncxx::builder::stream::close_document <<bsoncxx::builder::stream::finalize,
						bsoncxx::builder::stream::document{} << "$set"<< bsoncxx::builder::stream::open_document << "hora" << doc << bsoncxx::builder::stream::close_document <<bsoncxx::builder::stream::finalize);
			}
		}catch(const parse_error&){
			std::cout << "Python initialization error" << std::endl;
		};
	}
	return 0;
}

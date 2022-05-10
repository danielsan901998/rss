#include "parse.hpp"

#include <tinyxml2.h>
#include <iostream>
#include <iomanip>
#include <vector>

const tinyxml2::XMLElement* getroot(const tinyxml2::XMLDocument& parser){
    const tinyxml2::XMLElement* pNode = parser.FirstChildElement();
    return pNode;
}
std::string getcontent(const tinyxml2::XMLElement* node){
    return "";
}
std::string parseblog(const std::string& xml, const std::string& last, const std::string& contain){
    std::string first="";
    std::vector<std::string> links;
    tinyxml2::XMLDocument doc;
    doc.Parse(xml.c_str());
    const tinyxml2::XMLElement* node = getroot(doc);
    if(node==nullptr)return first;
    return first;
}
std::string parsepodcast(const std::string& xml, const std::string& last){
}
std::time_t parseyoutube(const std::string& xml, std::time_t last,const bsoncxx::v_noabi::document::view& doc ){
}

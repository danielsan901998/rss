#include "parse.hpp"

#include <tinyxml2.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>

std::string getcontent(const tinyxml2::XMLElement* element){
    std::string content;
    const char* tmp=element->GetText();
    if(tmp){
        return tmp;
    }
    const tinyxml2::XMLAttribute * attribute = element->FindAttribute("href");
    if(attribute)
        return attribute->Value();
    attribute = element->FindAttribute("url");
    if(attribute)
        return attribute->Value();
    return content;
}

const tinyxml2::XMLElement* getroot(const tinyxml2::XMLDocument& doc){
    if(!doc.Error()){
        const tinyxml2::XMLElement* element = doc.FirstChildElement();
        element = doc.FirstChildElement("feed");
        if(element)
            return element;
        element = doc.FirstChildElement("channel");
        if(element)
            return element;
        element = doc.FirstChildElement("rss");
        if(element)
            return element->FirstChildElement();
    }
    return nullptr;
}

std::string parseblog(const std::string& xml, const std::string& last, const std::string& contain){
    std::string first;
    std::vector<std::string> links;
    tinyxml2::XMLDocument doc;
    doc.Parse(xml.c_str());
    const tinyxml2::XMLElement* root = getroot(doc);
    if(root==nullptr)return first;
    for(auto item = root->FirstChildElement();item;item=item->NextSiblingElement()){
        const std::string nodename = item->Name();
        if(nodename=="item" || nodename=="entry"){
            std::string title;
            std::string link;
            for(auto e = item->FirstChildElement();e;e=e->NextSiblingElement()){
                const std::string childname = e->Name();
                if(childname=="title") {
                    title=getcontent(e);
                    if(title==last){
                        std::reverse(links.begin(), links.end());
                        for(const std::string& l:links)
                            std::cout << l << std::endl;
                        return first;
                    }
                    if(first=="")first=title;
                }
                else if(childname=="link"){
                    link=getcontent(e);
                }
            }
            if(contain=="" || title.find(contain)!=std::string::npos){
                links.push_back(link);
            }
        }
    }
    return first;
}
std::string parsepodcast(const std::string& xml, const std::string& last){
    std::string first;
    std::vector<std::string> links;
    tinyxml2::XMLDocument doc;
    doc.Parse(xml.c_str());
    const tinyxml2::XMLElement* root = getroot(doc);
    if(root==nullptr)return first;
    for(auto item = root->FirstChildElement();item;item=item->NextSiblingElement()){
        const std::string nodename = item->Name();
        if(nodename=="item" || nodename=="entry"){
            std::string title;
            std::string link;
            for(auto e = item->FirstChildElement();e;e=e->NextSiblingElement()){
                const std::string childname = e->Name();
                if(childname=="title") {
                    title=getcontent(e);
                    if(first=="")first=title;
                    if(title==last) return first;
                }
                else if(childname=="link" || childname=="enclosure"){
                    link=getcontent(e);
                }
            }
            std::replace( title.begin(), title.end(), '/', '-');
            download(link, "/home/daniel/videos/podcast/"+title+".mp3");
        }
    }
    return first;
}
std::time_t parseyoutube(const std::string& xml, std::time_t last,const bsoncxx::v_noabi::document::view& doc_view ){
    tinyxml2::XMLDocument doc;
    doc.Parse(xml.c_str());
    const tinyxml2::XMLElement* root = getroot(doc);
    if(root==nullptr)return std::time_t(0);
    std::string command="~/bin/youtube -q";
    std::string output=" > /dev/null";
    std::string folder="";
    std::time_t first=last;
    std::vector<std::string> contain;
    std::vector<std::string> notcontain;
    std::string urls="";
    if(doc_view["regex"]){
        bsoncxx::document::element reg = doc_view["regex"];
        if(reg["true"])for (const bsoncxx::array::element& msg : reg["true"].get_array().value) {
            contain.push_back(msg.get_utf8().value.to_string());
        }
        else if(reg["false"])for (const bsoncxx::array::element& msg : reg["false"].get_array().value) {
            notcontain.push_back(msg.get_utf8().value.to_string());
        }
    }
    std::vector<std::string> links;
    for(auto item = root->FirstChildElement();item;item=item->NextSiblingElement()){
        const std::string nodename = item->Name();
        if(nodename=="entry"){
            std::string title;
            std::string link;
            std::tm t = {};
            std::time_t time;
            for(auto e = item->FirstChildElement();e;e=e->NextSiblingElement()){
                const std::string childname = e->Name();
                if(childname=="published"){
                    std::istringstream ss(e->Value());
                    ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S+00:00");
                    time = mktime(&t);
                    if(time<=last)
                    {
                        if(urls!=""){
                            int ret=system((command+urls+folder+output).c_str());
                            if(ret==-1)
                                std::cout << "error en llamada a system: " << (command+urls+folder+output) << std::endl;
                        }
                        return first;
                    }
                    else if(first==last) first=time;
                }
                else if(childname=="title") {
                    title=e->Value();
                }
                else if(childname=="link"){
                    link=getcontent(e);
                }
            }
            bool descargar=true;
            if(contain.size()!=0){
                descargar=false;
                for(std::string& regex: contain) 
                    if(title.find(regex)!=std::string::npos)descargar=true;
            }
            else if(notcontain.size()!=0){
                for(std::string& regex: notcontain) 
                    if(title.find(regex)!=std::string::npos)descargar=false;
            }
            if(descargar){
                urls+=" ";
                urls+=link;
            }
        }
    }
    return std::time_t(0);
}

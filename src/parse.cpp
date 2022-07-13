#include "parse.hpp"

#include <tinyxml2.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <filesystem>
#include <Python.h>
std::filesystem::path home = getenv("HOME");
Python::Python(){
    Py_Initialize();
    std::string path=home / "scripts/rss";
    PyObject* sysPath = PySys_GetObject("path");
    PyObject* programName = PyUnicode_FromString(path.c_str());
    PyList_Append(sysPath, programName);
    pluginModule = PyImport_ImportModule("youtube");

    if (pluginModule == nullptr){
        PyErr_Print();
        throw parse_error();
    }
    // pDict is a borrowed reference
    pDict = PyModule_GetDict(pluginModule);
    // pFunc is also a borrowed reference
    pFunc = PyDict_GetItemString(pDict, "download");

    //clean up
    //PyRun_SimpleString("import youtube");
    Py_DECREF(programName);
}
Python::~Python(){
    Py_DECREF(pluginModule);
    Py_Finalize();
}
void Python::download(std::string folder, const std::vector<std::string> urls){
    folder+="/";
    PyObject* pyfolder = PyUnicode_FromString(folder.c_str());
    PyObject* pyurls = PyList_New(urls.size());
    for(size_t i = 0; i<urls.size();i++)
        PyList_SetItem(pyurls, i, PyUnicode_FromString(urls[i].c_str()));
    PyObject* args = PyTuple_Pack(3,Py_True,pyfolder,pyurls);
    PyObject_CallObject(pFunc,args);
    Py_DECREF(args);
    Py_DECREF(pyurls);
    //removed because cause segfault.
    //Py_DECREF(pyfolder);
}

std::string getcontent(const tinyxml2::XMLElement* element){
    std::string content;
    const char* tmp=element->GetText();
    if(tmp)
        return tmp;
    const tinyxml2::XMLAttribute* attribute = element->FindAttribute("href");
    if(attribute)
        return attribute->Value();
    attribute = element->FindAttribute("url");
    if(attribute)
        return attribute->Value();
    return content;
}

const tinyxml2::XMLElement* getroot(const tinyxml2::XMLDocument& doc){
    if(!doc.Error()){
        const tinyxml2::XMLElement* element = doc.FirstChildElement("feed");
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
            download(link, home / "videos/podcast"/title+=".mp3");
        }
    }
    return first;
}
std::time_t Youtube_Parse::parseyoutube(const std::string& xml, std::time_t last,const bsoncxx::v_noabi::document::view& doc_view ){
    tinyxml2::XMLDocument doc;
    doc.Parse(xml.c_str());
    const tinyxml2::XMLElement* root = getroot(doc);
    if(root==nullptr)return std::time_t(0);
    std::string folder;
    std::time_t first=last;
    std::vector<std::string> contain;
    std::vector<std::string> notcontain;
    std::vector<std::string> urls;
    if(doc_view["regex"]){
        bsoncxx::document::element reg = doc_view["regex"];
        if(reg["true"])for (const bsoncxx::array::element& msg : reg["true"].get_array().value) {
            contain.push_back(msg.get_utf8().value.to_string());
        }
        else if(reg["false"])for (const bsoncxx::array::element& msg : reg["false"].get_array().value) {
            notcontain.push_back(msg.get_utf8().value.to_string());
        }
    }
    if(doc_view["folder"]){
        folder=doc_view["folder"].get_utf8().value.to_string();
    }
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
                    std::istringstream ss(getcontent(e));
                    ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S+00:00");
                    time = mktime(&t);
                    if(time<=last)
                    {
                        if(!urls.empty())
                            py.download(folder,urls);
                        return first;
                    }
                    else if(first==last) first=time;
                }
                else if(childname=="title") {
                    title=getcontent(e);
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
                urls.push_back(link);
            }
        }
    }
    return std::time_t(0);
}

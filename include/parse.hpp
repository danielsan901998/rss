#pragma once
#include "request.hpp"

#include <bsoncxx/builder/stream/document.hpp>
#include <string>
#include <Python.h>

std::string parseblog(const std::string& xml, const std::string& last, const std::string& contain);
std::string parsepodcast(const std::string& xml, const std::string& last);
std::time_t parseyoutube(const std::string& xml, std::time_t last,const bsoncxx::v_noabi::document::view& doc_view );
class parse_error{};
class Python{
    PyObject* pluginModule;
    PyObject* pDict;
    PyObject* pFunc;
    public:
    Python();
    ~Python();
    void download(std::string folder, const std::vector<std::string>& urls);
};

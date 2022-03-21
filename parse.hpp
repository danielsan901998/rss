#pragma once
#include "request.hpp"

#include <libxml++/libxml++.h>
#include <bsoncxx/builder/stream/document.hpp>

#include <string>
#include <iostream>
#include <iomanip>

std::string parseblog(const std::string& xml, const std::string& last, const std::string& contain);
std::string parsepodcast(const std::string& xml, const std::string& last);
std::time_t parseyoutube(const std::string& xml, std::time_t last, const bsoncxx::v_noabi::document::view& doc);

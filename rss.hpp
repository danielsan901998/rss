#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <algorithm>
#include <regex>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

#include <libxml++/libxml++.h>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>


std::string request(const std::string& string);
void download(const std::string& url, const std::string& filename);
std::string parseblog(const std::string& xml, const std::string& last, const std::string& contain);
std::string parsepodcast(const std::string& xml, const std::string& last);
std::time_t parseyoutube(const std::string& xml, std::time_t last, const bsoncxx::v_noabi::document::view& doc);

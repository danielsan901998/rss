#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <algorithm>

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
std::string request(const std::string& string, const std::string&& agent);
void download(const std::string& string, const std::string& filename);
std::string parseImmortal(const std::string& xml, const std::string& last);
std::string parseblog(const std::string& xml, const std::string& last);
std::string parsepodcast(const std::string& xml, const std::string& last);
std::time_t parseyoutube(const std::string& xml, std::time_t last, std::string nombre);

#pragma once
#include <string>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

#include <sstream>
#include <fstream>

std::string request(const std::string& string);
void download(const std::string& url, const std::string& filename);

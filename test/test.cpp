#include <request.hpp>
#include <parse.hpp>

#include <string>
#include <cassert>
#include <fstream>
#include <sstream>

std::string read_file(const char* path)
{
	std::string data;
	std::ifstream t(path);
	if(t.is_open())
	{
		std::stringstream buffer;
		buffer << t.rdbuf();
		data  = buffer.str();
	}

	return data;
}

int main(){
	std::string xml=request("https://localhost/nonexistant");
	assert(xml.empty());
	xml=read_file("rss_test.xml");
	std::string last=parseblog(xml,"test2");
	assert(last=="test1");
	return 0;
}

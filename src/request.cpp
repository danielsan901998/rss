#include "request.hpp"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

#include <sstream>
#include <fstream>

std::string request(const std::string& string){
	std::ostringstream os;
	try
	{
		curlpp::Cleanup myCleanup;
		curlpp::Easy myRequest;
		myRequest.setOpt<curlpp::options::Url>(string);
		myRequest.setOpt<curlpp::options::FollowLocation>(true);
		myRequest.setOpt<curlpp::options::MaxRedirs>(10);
		myRequest.setOpt<curlpp::options::UserAgent>("curl");
		myRequest.setOpt<curlpp::options::WriteStream>(&os);
		myRequest.perform();
		long code = curlpp::infos::ResponseCode::get(myRequest);
		if(code==404){
			return "";
		}
		return os.str();
	}

	catch( curlpp::RuntimeError &e )
	{
		std::cerr << e.what() << std::endl;
	}

	catch( curlpp::LogicError &e )
	{
		std::cerr << e.what() << std::endl;
	}
	return os.str();
}
void download(const std::string& url, const std::string& filename){
	std::ofstream os(filename);
	try
	{
		curlpp::Cleanup myCleanup;
		curlpp::Easy myRequest;
		myRequest.setOpt(curlpp::options::Url(url));
		myRequest.setOpt(curlpp::options::FollowLocation(true));
		os << myRequest;
		return;
	}

	catch( curlpp::RuntimeError &e )
	{
		std::cerr << e.what() << std::endl;
	}

	catch( curlpp::LogicError &e )
	{
		std::cerr << e.what() << std::endl;
	}
}

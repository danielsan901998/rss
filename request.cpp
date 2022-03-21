#include "request.hpp"

std::string request(const std::string& string){
    std::ostringstream os;
    try
    {
        curlpp::Cleanup myCleanup;
        curlpp::Easy myRequest;
        myRequest.setOpt<curlpp::options::Url>(string);
        myRequest.setOpt<curlpp::options::FollowLocation>(true);
        myRequest.setOpt<curlpp::options::SslVerifyPeer>(false);
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
        std::cout << e.what() << std::endl;
    }

    catch( curlpp::LogicError &e )
    {
        std::cout << e.what() << std::endl;
    }
    return os.str();
}
void download(const std::string& url, const std::string& filename){
    std::ofstream os(filename);
    try
    {
        curlpp::Cleanup myCleanup;
        curlpp::Easy request;
        request.setOpt(new curlpp::options::Url(url));
        request.setOpt(new curlpp::options::FollowLocation(true));
        os << request;
        return;
    }

    catch( curlpp::RuntimeError &e )
    {
        std::cout << e.what() << std::endl;
    }

    catch( curlpp::LogicError &e )
    {
        std::cout << e.what() << std::endl;
    }
}

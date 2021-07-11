#include "rss.hpp"

std::string request(const std::string& string)
{
    std::ostringstream os;
    try
    {
        curlpp::Cleanup myCleanup;
        curlpp::Easy myRequest;
        myRequest.setOpt<curlpp::options::Url>(string);
        myRequest.setOpt<curlpp::options::FollowLocation>(true);
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
void download(const std::string& url, const std::string& filename)
{
    std::ofstream os(filename);
    try
    {
        curlpp::Cleanup myCleanup;
        curlpp::Easy request;
        request.setOpt(new curlpp::options::Url(url));
        request.setOpt(new curlpp::options::NoBody(true));
        request.perform();
        char* redirect;
        long code = curlpp::infos::ResponseCode::get(request);
        if(code==200){
            os << curlpp::options::Url(url);
            return;
        }
        if(code==301 || code==302){
            curlpp::InfoGetter::get(request,CURLINFO_REDIRECT_URL, redirect);
            download(redirect,filename);
            return;
        }
        else{
            std::cout << "error" << std::endl;
            return;
        }
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

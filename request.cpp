#include "rss.hpp"

std::string request(const std::string& string)
{
    std::ostringstream os;
    try
    {
        curlpp::Cleanup myCleanup;

        {
            os << curlpp::options::Url(string);
            return os.str();
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
    return os.str();
}
std::string request(const std::string& string, const std::string&& agent)
{
    std::ostringstream os;
    try
    {
        curlpp::Cleanup myCleanup;
        curlpp::Easy myRequest;
        myRequest.setOpt<curlpp::options::Url>(string);
        myRequest.setOpt<curlpp::options::UserAgent>(agent);
        myRequest.setOpt<curlpp::options::WriteStream>(&os);
        myRequest.perform();
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
void download(const std::string& string, const std::string& filename)
{
    std::ofstream os(filename);
    try
    {
        curlpp::Cleanup myCleanup;

        {
            os << curlpp::options::Url(string);
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

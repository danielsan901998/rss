#include "rss.hpp"
std::string getcontent(const xmlpp::Node* node){
    const auto nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
    if(nodeContent){
        return nodeContent->get_content();
    }
    const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
    if(nodeElement)
        for (const auto& attribute : nodeElement->get_attributes()){
            if(attribute->get_name()=="href" || attribute->get_name()=="url"){
                return attribute->get_value();
            }
        }
    for(const auto& item : node->get_children()){
        const auto nodeText = dynamic_cast<const xmlpp::TextNode*>(item);
        if(nodeText){
            return nodeText->get_content();
        }
        else{
            return getcontent(item);
        }
    }
    return "";
}
std::string parseImmortal(const xmlpp::Node* node, std::string last)
{
    std::string first="";
    for(const auto& item : node->get_children())
    {
        const auto nodename = item->get_name();
        if(nodename=="item"){
            std::string title;
            std::string link;
            for(const auto& child : item->get_children())
            {
                const auto childname = child->get_name();
                if(childname=="title") {
                    title=getcontent(child);
                    if(first=="")first=title;
                    if(title==last) return first;
                }
                else if(childname=="link"){
                    link=getcontent(child);
                }
            }
            if(title.find("To You, The Immortal")!=std::string::npos){
                std::cout << link << std::endl;
            }
        }
    }
    return first;
}
std::string parseblog(const xmlpp::Node* node, const std::string&& entry, std::string last)
{
    std::string first="";
    for(const auto& item : node->get_children())
    {
        const auto nodename = item->get_name();
        if(nodename==entry){
            std::string title;
            std::string link;
            for(const auto& child : item->get_children())
            {
                const auto childname = child->get_name();
                if(childname=="title") {
                    title=getcontent(child);
                    if(first=="")first=title;
                    if(title==last) return first;
                }
                else if(childname=="link"){
                    link=getcontent(child);
                }
            }
            std::cout << link << std::endl;
        }
    }
    return first;
}
std::string parsepodcast(const xmlpp::Node* node, std::string last)
{
    std::string first="";
    for(const auto& item : node->get_children())
    {
        const auto nodename = item->get_name();
        if(nodename=="item"){
            std::string title;
            std::string link;
            std::string enclosure="";
            for(const auto& child : item->get_children())
            {
                const auto childname = child->get_name();
                if(childname=="title") {
                    title=getcontent(child);
                    if(first=="")first=title;
                    if(title==last) return first;
                }
                else if(childname=="link"){
                    link=getcontent(child);
                }
                else if(childname=="enclosure"){
                    link=getcontent(child);
                }
            }
            std::replace( title.begin(), title.end(), '/', '-');
            download(link, title+".mp3");
        }
    }
    return first;
}
std::time_t parseyoutube(const xmlpp::Node* node,std::time_t last)
{
    for(const auto& item : node->get_children())
    {
        const auto nodename = item->get_name();
        if(nodename=="entry"){
            std::string title;
            std::string link;
            std::tm t = {};
            std::time_t time;
            for(const auto& child : item->get_children())
            {
                const auto childname = child->get_name();
                if(childname=="published"){
                    std::istringstream ss(getcontent(child));
                    ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S+00:00");
                    time = mktime(&t);
                }
                else if(childname=="title") {
                    title=getcontent(child);
                }
                else if(childname=="link"){
                    link=getcontent(child);
                }
            }
            std::cout << title << std::endl;
            std::cout << link << std::endl;
            system((std::string("./youtube ")+link).c_str());
            if(time<=last)return time;
        }
    }
    return std::time_t(0);
}
std::string parserss(const std::string& xml, collection col, std::string last)
{
    try
    {
        xmlpp::DomParser parser;
        parser.parse_memory(xml);
        if(parser)
        {
            const xmlpp::Node* pNode = parser.get_document()->get_root_node();
            const xmlpp::Node* node;
            if(pNode->get_name()=="feed"){
                node = pNode;
            }
            else{
                for(const xmlpp::Node* child : pNode->get_children())
                {
                    const auto nodename = child->get_name();
                    if(nodename=="channel"){
                        node = child;
                        break;
                    }
                }
            }
            switch(col){
                case collection::ciceron:
                    return parseblog(node,"entry", last);
                case collection::inmortal:
                    return parseImmortal(node, last);
                case collection::blog:
                    return parseblog(node,"item", last);
                case collection::podcast:
                    return parsepodcast(node, last);
            }
        }
        return "";
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
        return "";
    }
}
std::time_t parserss(const std::string& xml, std::time_t last)
{
    try
    {
        xmlpp::DomParser parser;
        parser.parse_memory(xml);
        if(parser)
        {
            const xmlpp::Node* pNode = parser.get_document()->get_root_node();
            if(pNode->get_name()=="feed"){
                return parseyoutube(pNode, last);
            }
        }
        return std::time_t(0);
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
        return std::time_t(0);
    }
}

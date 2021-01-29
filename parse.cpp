#include "rss.hpp"
const xmlpp::Node* getroot(const xmlpp::DomParser& parser)
{
    if(parser){
        const xmlpp::Node* node;
        const xmlpp::Node* pNode = parser.get_document()->get_root_node();
        if(pNode->get_name()=="feed"){
            return pNode;
        }
        else{
            for(const xmlpp::Node* child : pNode->get_children())
            {
                const auto nodename = child->get_name();
                if(nodename=="channel"){
                    return child;
                }
            }
        }
    }
    return nullptr;
}
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
std::string parseblog(const std::string& xml, const std::string& last, const std::string& contain)
{
    std::string first="";
    try
    {
        xmlpp::DomParser parser;
        parser.parse_memory(xml);
        const xmlpp::Node* node = getroot(parser);
        if(node==nullptr)return first;
        for(const auto& item : node->get_children())
        {
            const auto nodename = item->get_name();
            if(nodename=="item" || nodename=="entry"){
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
                if(contain=="" || title.find(contain)!=std::string::npos){
                    std::cout << link << std::endl;
                }
            }
        }
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
    }
    return first;
}
std::string parsepodcast(const std::string& xml, const std::string& last)
{
    std::string first="";
    try
    {
        xmlpp::DomParser parser;
        parser.parse_memory(xml);
        const xmlpp::Node* node = getroot(parser);
        if(node==nullptr)return first;
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
                download(link, "~/videos/podcast/"+title+".mp3");
            }
        }

    }
    catch(const std::exception& ex)
    {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
    }
    return first;
}
std::time_t parseyoutube(const std::string& xml, std::time_t last, std::string nombre,const bsoncxx::v_noabi::document::view& doc )
{
    try
    {
	xmlpp::DomParser parser;
	parser.parse_memory(xml);
	const xmlpp::Node* node = getroot(parser);
        if(node==nullptr)return std::time_t(0);
        std::string command="~/bin/youtube ";
        std::string output=">> ~/youtube.log";
        std::string folder="";
        std::time_t first=last;
        std::vector<std::string> contain;
        std::vector<std::string> notcontain;
        if(doc["regex"]){
            bsoncxx::document::element reg = doc["regex"];
            if(reg["true"])for (const bsoncxx::array::element& msg : reg["true"].get_array().value) {
                contain.push_back(msg.get_utf8().value.to_string());
            }
            else if(reg["false"])for (const bsoncxx::array::element& msg : reg["false"].get_array().value) {
                notcontain.push_back(msg.get_utf8().value.to_string());
            }
        }
        if(doc["folder"]){
            folder=doc["folder"].get_utf8().value.to_string();
        }

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
                        if(time<=last)return first;
                        else if(first==last) first=time;
                    }
                    else if(childname=="title") {
                        title=getcontent(child);
                    }
                    else if(childname=="link"){
                        link=getcontent(child);
                    }
                }
                bool descargar=true;
                if(contain.size()!=0){
                    descargar=false;
                    for(std::string& regex: contain) 
                        if(title.find(regex)!=std::string::npos)descargar=true;
                }
                else if(notcontain.size()!=0){
                    for(std::string& regex: notcontain) 
                        if(title.find(regex)!=std::string::npos)descargar=false;
                }
                if(descargar){
                    system((command+link+" "+folder+output).c_str());
                }
            }
        }
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
    }
    return std::time_t(0);
}

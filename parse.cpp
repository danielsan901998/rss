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
std::string parseImmortal(const xmlpp::Node* node, const std::string& last)
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
std::string parseblog(const xmlpp::Node* node, const std::string&& entry, const std::string& last)
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
std::string parsepodcast(const xmlpp::Node* node, const std::string& last)
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
            download(link, "~/videos/podcast/"+title+".mp3");
        }
    }
    return first;
}
std::time_t parseyoutube(const xmlpp::Node* node, std::time_t last, std::string nombre)
{
    std::string command="~/bin/youtube ";
    std::string output=">> ~/youtube.txt";
    bool check=false;
    std::time_t first=last;
    if(nombre=="alexelcapo")check=true;
    else if(nombre=="Adult Swim")check=true;
    else if(nombre=="Pazos64")check=true;
    else if(nombre=="Accursed Farms")check=true;
    else if(nombre=="Cinemassacre")check=true;
    else if(nombre=="The Majority Report w/ Sam Seder")check=true;
    else if(nombre=="The Michael Brooks Show")check=true;
    else if(nombre=="FilmJoy")check=true;
    else if(nombre=="ElRichMC - Minecraft & Gaming a otro nivel")check=true;
    else if(nombre=="LowkoTV")check=true;
    else if(nombre=="Escapist")check=true;

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
                }
                else if(childname=="title") {
                    title=getcontent(child);
                }
                else if(childname=="link"){
                    link=getcontent(child);
                }
            }
            if(check){
                if(nombre=="alexelcapo"){
                    if(title.find("ENELSMADLH")!=std::string::npos)system((command+link+" resto"+output).c_str());
                }
                else if(nombre=="Adult Swim"){
                    if(title.find("Off the Air")!=std::string::npos)system((command+link+" ver"+output).c_str());
                }
                else if(nombre=="Pazos64"){
                    if(title.find("Incluso de Videojuegos")==std::string::npos)system((command+link+" ver"+output).c_str());
                }
                else if(nombre=="Accursed Farms"){
                    if(title.find("Videochat")==std::string::npos)system((command+link+" ver"+output).c_str());
                }
                else if(nombre=="Cinemassacre"){
                    if(title.find("Angry Video Game Nerd")!=std::string::npos)system((command+link+" ver"+output).c_str());
                }
                else if(nombre=="The Majority Report w/ Sam Seder"){
                    if(title.find("MR ")==std::string::npos)system((command+link+" podcast"+output).c_str());
                }
                else if(nombre=="The Michael Brooks Show"){
                    if(title.find("TMBS -")==std::string::npos)system((command+link+" podcast"+output).c_str());
                }
                else if(nombre=="FilmJoy"){
                    if(title.find("Movies with Mikey")!=std::string::npos)system((command+link+" ver"+output).c_str());
                }
                else if(nombre=="ElRichMC - Minecraft & Gaming a otro nivel"){
                    if(title.find("Minecraft Review")!=std::string::npos)system((command+link+" ver"+output).c_str());
                    else if(title.find("Hardcore Flatlands")!=std::string::npos)system((command+link+" ver"+output).c_str());
                    else if(title.find("Survival 1.7")!=std::string::npos)system((command+link+" ver"+output).c_str());
                }
                else if(nombre=="LowkoTV"){
                    if(title.find("StarCraft")!=std::string::npos)system((command+link+" ver"+output).c_str());
                }
                else if(nombre=="Escapist"){
                    if(title.find("The Big Picture")!=std::string::npos)system((command+link+" ver"+output).c_str());
                    else if(title.find("Zero Punctuation")!=std::string::npos)system((command+link+" ver"+output).c_str());
                    else if(title.find("Yahtzee's Dev Diary")!=std::string::npos)system((command+link+" ver"+output).c_str());
                }

            }
            else{
                system((command+link+output).c_str());
            }
        }
    }
    return std::time_t(0);
}
std::string parserss(const std::string& xml, collection col, const std::string& last)
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
std::time_t parserss(const std::string& xml, std::time_t last, const std::string& nombre)
{
    try
    {
        xmlpp::DomParser parser;
        parser.parse_memory(xml);
        if(parser)
        {
            const xmlpp::Node* pNode = parser.get_document()->get_root_node();
            if(pNode->get_name()=="feed"){
                return parseyoutube(pNode, last, nombre);
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

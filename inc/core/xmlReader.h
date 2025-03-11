#ifndef XMLREADER_H
#define XMLREADER_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include "tinyxml2.h"

namespace manifest {

namespace detail {
    bool isGitHash(const std::string& revision);

    bool isGitTag(const std::string& revision);

    bool isBranchName(std::string revision, bool debug = false);

    std::string getBranchName(std::string revision);
}

class XMLReader
{
public:
    XMLReader(std::string basePath, std::string baseFile);

    std::string getName(tinyxml2::XMLElement* element);
    std::string getNameFillRemote(tinyxml2::XMLElement* element);
    std::string getNameFillFetch(tinyxml2::XMLElement* element);
    std::string getPath(tinyxml2::XMLElement* element);
    std::string getPathFillRemote(tinyxml2::XMLElement* element);
    std::string getRemote(tinyxml2::XMLElement* element);
    std::string getRemoteUrl(tinyxml2::XMLElement* element);
    std::string getRevision(tinyxml2::XMLElement* element);
    std::string getUpstream(tinyxml2::XMLElement* element);
    std::string getBranchName(tinyxml2::XMLElement* element);
    std::vector<tinyxml2::XMLElement*> getProjectList();

    // 将目标XML归并于一个文件
    // 相当于执行 repo manifest -m xxx.xml -o xxx.xml
    void saveAsXML(std::string fileName, bool lossRemote = false);

private:
    // 如果一个project属于notdefault组，则，repo sync时不会下载
    // 默认不包含notdefault组,
    void init();

    void loadXMLDoc(std::string fileName);

    void addXMLRemote(tinyxml2::XMLElement* element);

    void addXMLDefault(tinyxml2::XMLElement* element);

    void addXMLElement(tinyxml2::XMLElement* element);

    void removeXMLElement(std::string projectName);

private:
    bool bInit_;
    std::string basePath_;
    std::string baseFile_;
    tinyxml2::XMLDocument xmlDoc_;

    std::string remoteDef_;
    std::string revisionDef_;
    std::map<std::string, std::string> remoteMap_;
    std::list<tinyxml2::XMLElement*> elements_;
};
}

#endif // XMLREADER_H

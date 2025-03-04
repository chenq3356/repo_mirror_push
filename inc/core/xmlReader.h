#ifndef XMLREADER_H
#define XMLREADER_H

#include <string>
#include <list>
#include <vector>
#include "tinyxml2.h"

struct ProjectInfo
{
    bool isFullPath;
    std::string name_; // git的名称，用于生成git url，URL格式是：${remote fetch}/${project name}.git
    std::string path_; // clone到本地的git的工作目录，如果没有配置的话，跟name一样
    std::string branch_; // 远程分支名
};

class XMLReader
{
public:
    XMLReader(std::string basePath, std::string baseFile);

    std::vector<ProjectInfo> getProjects();

    // 将目标XML归并于一个文件
    // 相当于执行 repo manifest -m xxx.xml -o xxx.xml
    void saveAsXML(std::string fileName, bool lossRemote = false);

    static bool isBranchName(std::string revision, bool debug = false);

private:
    // 如果一个project属于notdefault组，则，repo sync时不会下载
    // 默认不包含notdefault组,
    void init();

    void loadXMLDoc(std::string fileName);

    void addXMLElement(tinyxml2::XMLElement* element);

    void removeXMLElement(std::string projectName);

    static bool isGitHash(const std::string& revision);

    static bool isGitTag(const std::string& revision);

    std::string getBranchName(std::string revision);

    std::string parseBranchName(std::string revision, std::string upstream);

private:
    bool bInit_;
    std::string basePath_;
    std::string baseFile_;
    tinyxml2::XMLDocument xmlDoc_;
    std::list<tinyxml2::XMLElement*> elements_;
};

#endif // XMLREADER_H

#include "xmlReader.h"
//#include <regex>
#include <map>

namespace manifest {

namespace detail {
bool isGitHash(const std::string& revision) {
    // Git 对象哈希值是 40 个字符的十六进制字符串
    if (revision.length() != 40) {
        return false;
    }

    // 检查每个字符是否是十六进制字符
    for (char c : revision) {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    return true;
}
bool isGitTag(const std::string& revision) {
    if (std::string::npos != revision.find(".")) {
        return true;
    }
    return false;
}
bool isBranchName(std::string revision, bool debug) {
    if (revision.empty()) {
        if (debug) {
            printf("revision is empty\n");
        }
        return false;
    }

    // 相对引用
    size_t pos = revision.find("HEAD~");
    if (std::string::npos != pos && 0 == pos) {
        if (debug) {
            printf("%s is HEAD~\n", revision.c_str());
        }
        return false;
    }

    // 明确指定标签
    pos = revision.find("refs/tags/");
    if (std::string::npos != pos) {
        if (debug) {
            printf("%s is refs/tags\n", revision.c_str());
        }
        return false;
    }
    // 明确指定分支
    pos = revision.find("refs/heads/");
    if (std::string::npos != pos) {
        if (debug) {
            printf("%s is refs/heads\n", revision.c_str());
        }
        return true;
    }
    // 用于 Gerrit 代码评审系统
    pos = revision.find("refs/changes/");
    if (std::string::npos != pos) {
        if (debug) {
            printf("%s is refs/changes\n", revision.c_str());
        }
        return false;
    }

    // 哈希
    if (isGitHash(revision)) {
        if (debug) {
            printf("%s is hash\n", revision.c_str());
        }
        return false;
    }
    // 标签
    if (isGitTag(revision)) {
        if (debug) {
            printf("%s is tag\n", revision.c_str());
        }
        return false;
    }

    if (debug) {
        printf("%s is branch name\n", revision.c_str());
    }

    return true;
}
std::string getBranchName(std::string revision)
{
    size_t pos = revision.find("refs/heads/");
    if (std::string::npos != pos) {
        return revision.substr(pos+11);
    }
    return revision;
}
}


XMLReader::XMLReader(std::string basePath, std::string baseFile)
{
    bInit_    = false;
    basePath_ = basePath;
    baseFile_ = baseFile;
}

void XMLReader::init()
{
    if (bInit_) {
        return;
    }

    bInit_ = true;
    loadXMLDoc(baseFile_);
}

void XMLReader::loadXMLDoc(std::string fileName)
{
    tinyxml2::XMLDocument xmlObj;

    std::string fullpath = basePath_ + std::string("/") + fileName;
    tinyxml2::XMLError errCode = xmlObj.LoadFile(fullpath.c_str());
    if(errCode != tinyxml2::XML_SUCCESS){
        printf("LoadFile %s failed, errCode = %d\n", fileName.c_str(), errCode);
        exit(0);
    }

    // 获取根节点
    tinyxml2::XMLElement* root = xmlObj.RootElement();
    if (!root) {
        printf("Invalid document structure.\n");
        exit(0);
    }

    for (tinyxml2::XMLElement* currenteleElement = root->FirstChildElement(); currenteleElement; currenteleElement = currenteleElement->NextSiblingElement()) {
        if (NULL == currenteleElement->Name()) {
            printf("%s: currentele Element Name Is Null\n", __FUNCTION__);
            exit(0);
        }

        std::string elementName = currenteleElement->Name();
        if ("include" == elementName) {
            loadXMLDoc(currenteleElement->Attribute("name"));
        } else if ("remove-project" == elementName) {
            removeXMLElement(currenteleElement->Attribute("name"));
        } else if ("remote" == elementName) {
            addXMLRemote(currenteleElement);
        }  else if ("default" == elementName) {
            addXMLDefault(currenteleElement);
        } else if ("project" == elementName) {
            addXMLElement(currenteleElement);
        } else {
            addXMLElement(currenteleElement);
        }
    }
}

void XMLReader::addXMLRemote(tinyxml2::XMLElement* element)
{
    if (!element->Attribute("name")) {
        return;
    }

    if (!element->Attribute("fetch")) {
        return;
    }

    // 删除所有末尾的'/'
    std::string fetch = element->Attribute("fetch");
    while (!fetch.empty() && fetch.back() == '/') {
        fetch.pop_back();
    }

    tinyxml2::XMLNode *copyNode = element->DeepClone(&xmlDoc_);
    tinyxml2::XMLElement* copyElement = copyNode->ToElement();
    if (copyElement) {
        elements_.push_back(copyElement);
        remoteMap_[element->Attribute("name")] = fetch;
    }
}

void XMLReader::addXMLDefault(tinyxml2::XMLElement* element)
{
    tinyxml2::XMLNode *copyNode = element->DeepClone(&xmlDoc_);
    tinyxml2::XMLElement* copyElement = copyNode->ToElement();
    if (copyElement) {
        if (element->Attribute("remote")) {
            remoteDef_ = std::string(element->Attribute("remote"));
        }
        if (element->Attribute("revision")) {
            revisionDef_ = std::string(element->Attribute("revision"));
        }
        elements_.push_back(copyElement);
    }
}

void XMLReader::addXMLElement(tinyxml2::XMLElement* element)
{
    const char* attr = element->Attribute("groups");
    // 不包含notdefault组
    if (!attr || NULL == strstr(attr, "notdefault")) {
        tinyxml2::XMLNode *copyNode = element->DeepClone(&xmlDoc_);
        tinyxml2::XMLElement* copyElement = copyNode->ToElement();
        if (copyElement) {
            elements_.push_back(copyElement);
        }
    }
}

void XMLReader::removeXMLElement(std::string projectName)
{
    auto iter = elements_.begin();
    while (iter != elements_.end()) {
        tinyxml2::XMLElement* currenteleElement = *iter;
        std::string elementName = currenteleElement->Name();
        if ("project" == elementName) {
            std::string attributeName = currenteleElement->Attribute("name");
            if (attributeName == projectName) {
                iter = elements_.erase(iter);
                xmlDoc_.DeleteChild(currenteleElement);
                break;
            } else {
                iter++;
            }
        } else {
            iter++;
        }
    }
}

std::string XMLReader::getName(tinyxml2::XMLElement* element) {
    return std::string(element->Attribute("name"));
}

std::string XMLReader::getNameFillRemote(tinyxml2::XMLElement* element) {
    std::string fullName = getRemote(element) + std::string("/") + getName(element);
    return fullName;
}

std::string XMLReader::getNameFillFetch(tinyxml2::XMLElement* element) {
    std::string fullName = getRemoteUrl(element) + std::string("/") + getName(element);
    return fullName;
}

std::string XMLReader::getPath(tinyxml2::XMLElement* element) {
    if (element->Attribute("path")) {
        return std::string(element->Attribute("path"));
    }
    return getName(element);
}

std::string XMLReader::getPathFillRemote(tinyxml2::XMLElement* element)
{
    std::string fullPath = getRemote(element) + std::string("/") + getPath(element);
    return fullPath;
}

std::string XMLReader::getRemote(tinyxml2::XMLElement* element) {
    if (element->Attribute("remote")) {
        return std::string(element->Attribute("remote"));
    }
    return remoteDef_;
}

std::string XMLReader::getRemoteUrl(tinyxml2::XMLElement* element)
{
    std::string remote = getRemote(element);
    if (remote.empty()) {
        return "";
    }
    return remoteMap_[remote];
}

std::string XMLReader::getRevision(tinyxml2::XMLElement* element) {
    if (element->Attribute("revision")) {
        return std::string(element->Attribute("revision"));
    }
    return revisionDef_;
}

std::string XMLReader::getUpstream(tinyxml2::XMLElement* element) {
    if (element->Attribute("upstream")) {
        return std::string(element->Attribute("upstream"));
    }
    return "";
}

std::string XMLReader::getBranchName(tinyxml2::XMLElement* element){
    std::string revision = getRevision(element);
    if (detail::isBranchName(revision)) {
        return detail::getBranchName(revision);
    }

    std::string upstream = getUpstream(element);
    if (detail::isBranchName(upstream)) {
        return detail::getBranchName(upstream);
    }
    return "master";
}

std::vector<tinyxml2::XMLElement*> XMLReader::getProjectList() {
    init();

    std::vector<tinyxml2::XMLElement*> vecProject;
    for (auto iter = elements_.begin(); iter != elements_.end(); iter++) {
        tinyxml2::XMLElement* element = *iter;
        std::string elementName = element->Name();
        if ("project" == elementName) {
            vecProject.push_back(element);
        }
    }
    return vecProject;
}

// repo manifest -m xxx.xml -o xxx.xml
void XMLReader::saveAsXML(std::string fileName, bool lossRemote)
{
    init();

    // 创建XML文档对象
    tinyxml2::XMLDocument doc;
    // 创建XML声明
    tinyxml2::XMLDeclaration* declaration = doc.NewDeclaration();
    doc.InsertEndChild(declaration);
    // 创建根元素
    tinyxml2::XMLElement* root = doc.NewElement("manifest");
    doc.InsertEndChild(root);

    for (auto iter = elements_.begin(); iter != elements_.end(); iter++) {
        // 拷贝子元素并添加到根元素中
        tinyxml2::XMLElement* element = *iter;
        std::string elementName = element->Name();
        if ("remote" == elementName) {
            tinyxml2::XMLNode *copyNode = element->DeepClone(&doc);
            tinyxml2::XMLElement* copyElement = copyNode->ToElement();
            copyElement->SetAttribute("fetch", copyElement->Attribute("name"));
            root->InsertEndChild(copyElement);
        } else if (lossRemote && "project" == elementName && element->Attribute("clone-depth")) {
            tinyxml2::XMLElement* copyElement = doc.NewElement(elementName.c_str());
            if (element->Attribute("groups")) {
                copyElement->SetAttribute("groups", element->Attribute("groups"));
            }
            if (element->Attribute("name")) {
                copyElement->SetAttribute("name", element->Attribute("name"));
            }
            if (element->Attribute("path")) {
                copyElement->SetAttribute("path", element->Attribute("path"));
            }
            copyElement->SetAttribute("revision", "refs/heads/master");
            root->InsertEndChild(copyElement);
        } else {
            tinyxml2::XMLNode *copyNode = element->DeepClone(&doc);
            root->InsertEndChild(copyNode);
        }
    }

    // 保存XML文档到文件
    doc.SaveFile(fileName.c_str());
}

}

#include "xmlReader.h"
#include <regex>
#include <map>

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
        } else if ("project" == elementName) {
            const char* attr = currenteleElement->Attribute("groups");
            // 不包含notdefault组
            if (!attr || NULL == strstr(attr, "notdefault")) {
                addXMLElement(currenteleElement);
            }
        } else {
            addXMLElement(currenteleElement);
        }
    }
}

void XMLReader::addXMLElement(tinyxml2::XMLElement* element)
{
    tinyxml2::XMLNode *copyNode = element->DeepClone(&xmlDoc_);

    tinyxml2::XMLElement* copyElement = copyNode->ToElement();
    if (copyElement) {
        std::string elementName = copyElement->Name();
        if ("remote" == elementName) {
            copyElement->SetAttribute("fetch", copyElement->Attribute("name"));
        }
        elements_.push_back(copyElement);
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

// repo manifest -m xxx.xml -o xxx.xml
void XMLReader::saveAsXML(std::string fileName)
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
        std::string elementName = (*iter)->Name();

        // 拷贝子元素并添加到根元素中
        tinyxml2::XMLNode *copyNode = (*iter)->DeepClone(&doc);
        root->InsertEndChild(copyNode);
    }

    // 保存XML文档到文件
    doc.SaveFile(fileName.c_str());
}

bool XMLReader::isGitHash(const std::string& str) {
    // 定义正则表达式：40 或 64 个十六进制字符
    std::regex hashRegex("^[0-9a-fA-F]{40}$|^[0-9a-fA-F]{64}$");

    // 使用正则表达式匹配
    return std::regex_match(str, hashRegex);
}

bool XMLReader::isGitTag(const std::string& revision) {
    // 定义正则表达式：匹配 Git 标签名称

    // ^[a-zA-Z0-9]：标签名称以字母或数字开头
    // [-_.]：必须包含至少一个特定符号(-_.)
    // [a-zA-Z0-9-_.]*：可以包含零个或多个字母或数字或符号
    // [a-zA-Z0-9]$：以字母或数字结尾
    std::regex tagRegex("^[a-zA-Z0-9][a-zA-Z0-9]*[-_.][a-zA-Z0-9-_.]*[a-zA-Z0-9]$");

    // 使用正则表达式匹配
    return std::regex_match(revision, tagRegex);
}

std::string XMLReader::parseRevision(std::string revision)
{
    if (revision.empty()) {
        return "master";
    }
    /*
    * 1、指定一个分支名称，表示使用该分支的最新提交 revision="main"
    * 2、指定提交的哈希 revision="a56e0e17e23f925ff44c75e5b89330ccc2598640"
    * 3、标签名称 revision="v1.0.0"
    * 4、相对引用 revision="HEAD~1"
    * 5、默认值，如果 revision 字段未指定，则使用 manifest 文件中的默认值（通过 <default> 标签指定）,在XMLReader::getProjects 中已处理
    * 6、明确指定标签 refs/tags/<tag>
    * 7、明确指定分支 refs/heads/<branch>
    * 8、用于 Gerrit 代码评审系统 refs/changes/<change>
    */

    // 相对引用
    size_t pos = revision.find("HEAD~");
    if (std::string::npos != pos && 0 == pos) {
        return "master";
    }

    // 明确指定标签
    pos = revision.find("refs/tags/");
    if (std::string::npos != pos) {
        return "master";
    }
    // 明确指定分支
    pos = revision.find("refs/heads/");
    if (std::string::npos != pos) {
        return revision.substr(pos+11);
    }
    // 用于 Gerrit 代码评审系统
    pos = revision.find("refs/changes/");
    if (std::string::npos != pos) {
        return "master";
    }

    // 哈希
    if (isGitHash(revision)) {
        return "master";
    }
    // 标签
    if (isGitTag(revision)) {
        return "master";
    }

    return revision;
}

std::vector<ProjectInfo> XMLReader::getProjects()
{
    init();

    std::string remoteDef;
    std::string revisionDef;
    for (auto iter = elements_.begin(); iter != elements_.end(); iter++) {
        tinyxml2::XMLElement* currenteleElement = *iter;
        std::string elementName = currenteleElement->Name();
        if ("default" == elementName) {
            const char* remoteAttr = currenteleElement->Attribute("remote");
            if (remoteAttr) {
                remoteDef = remoteAttr;
            }
            const char* revisionAttr = currenteleElement->Attribute("revision");
            if (revisionAttr) {
                revisionDef = revisionAttr;
            }
            break;
        }
    }

    std::vector<ProjectInfo> list;
    for (auto iter = elements_.begin(); iter != elements_.end(); iter++) {
        tinyxml2::XMLElement* currenteleElement = *iter;
        std::string elementName = currenteleElement->Name();
        if ("project" == elementName) {
            const char* nameAttr = currenteleElement->Attribute("name");
            if (!nameAttr) {
                printf("%s: currentele Element Name Is Null\n", __FUNCTION__);
                exit(0);
            }

            const char* pathAttr = currenteleElement->Attribute("path");
            const char* remoteAttr = currenteleElement->Attribute("remote");
            const char* revisionAttr = currenteleElement->Attribute("revision");

            ProjectInfo info;
            info.isFullPath = false;
            if (remoteAttr) {
                info.name_ = std::string(remoteAttr) + std::string("/") + std::string(nameAttr);
            } else {
                info.name_ = remoteDef + std::string("/") + std::string(nameAttr);
            }

            if (pathAttr) {
                info.path_ = std::string(pathAttr);
            } else {
                info.path_ = std::string(nameAttr);
            }

            std::string revision;
            if (revisionAttr) {
                revision = std::string(revisionAttr);
            } else {
                revision = revisionDef;
            }
            info.branch_ = parseRevision(revision);

            list.push_back(info);
        }
    }
    return list;
}



#include "xmlReader.h"
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

std::vector<ProjectInfo> XMLReader::getProjects()
{
    init();

    std::string remoteDef;
    for (auto iter = elements_.begin(); iter != elements_.end(); iter++) {
        tinyxml2::XMLElement* currenteleElement = *iter;
        std::string elementName = currenteleElement->Name();
        if ("default" == elementName) {
            const char* remoteAttr = currenteleElement->Attribute("remote");
            if (remoteAttr) {
                remoteDef = remoteAttr;
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
            list.push_back(info);
        }
    }
    return list;
}



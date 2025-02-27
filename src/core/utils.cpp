#include "utils.h"
#include <sstream>

namespace utils {

JsObj string2Json(const std::string &strJson)
{
    JsObj resjs;
    try {
        resjs = JsObj::parse(strJson);
    }
    catch (JsObj::parse_error &e) {
        return resjs;
    }
    return resjs;
}

bool isJsonArray(JsArry &json)
{
    return json.is_array();
}

int getJsonArraySize(JsArry &json)
{
    if (!isJsonArray(json)) {
        return 0;
    }

    return json.size();
}

JsObj getJsonArray(JsArry &json, int pos)
{
    if (getJsonArraySize(json) > pos) {
        return json[pos];
    }

    return JsObj();
}

bool isJsonObject(JsObj &json)
{
    return json.is_object();
}

int getJsonValueInt(JsObj &json, const std::string &key, int defVal)
{
    if (!json.is_object() || !json[key].is_number_integer())
    {
        return defVal;
    }
    return json.value(key, defVal);
}

bool getJsonValueBool(JsObj &json, const std::string &key, bool defVal)
{
    if (!json.is_object() || !json[key].is_boolean())
    {
        return defVal;
    }
    return json.value(key, defVal);
}

std::string getJsonValueString(JsObj &json, const std::string &key, std::string defVal)
{
    if (!json.is_object() || !json[key].is_string())
    {
        return defVal;
    }
    return json.value(key, defVal);
}

std::vector<std::string> splitPath(const std::string& path, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;
    while (std::getline(ss, part, delimiter)) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    return parts;
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string getFileName(const std::string& path) {
    // 查找最后一个路径分隔符
    size_t lastSlash = path.find_last_of("/\\");

    // 如果找到分隔符，返回分隔符后的部分；否则返回整个路径
    return (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
}


/*
// 分割路径

#include <iostream>

// 插入路径到目录树
void insertPath(TreeNode* root, const std::vector<std::string>& parts) {
    TreeNode* current = root;
    for (const std::string& part : parts) {
        if (current->children_.find(part) == current->children_.end()) {
            current->children_[part] = new TreeNode(part);
        }
        current = current->children_[part];
    }
}

// 打印目录树
void printTree(TreeNode* root, int level = 0) {
    for (int i = 0; i < level; ++i) {
        std::cout << "  ";
    }
    std::cout << root->name_ << std::endl;
    for (const auto& child : root->children_) {
        printTree(child.second, level + 1);
    }
}
void XMLReader::getOriginNode()
{
    init();

    // 创建根节点
    TreeNode* root = new TreeNode("");

    for (auto iter = elements_.begin(); iter != elements_.end(); iter++) {
        tinyxml2::XMLElement* currenteleElement = *iter;
        if (NULL == currenteleElement->Name()) {
            printf("%s: currentele Element Name Is Null\n", __FUNCTION__);
            exit(0);
        }

        std::string elementName = currenteleElement->Name();
        if ("project" == elementName) {
            std::string path = currenteleElement->Attribute("name");
            std::vector<std::string> parts = splitPath(path);
            insertPath(root, parts);
        } else {
            iter++;
        }
    }

    printTree(root);

}
*/

}

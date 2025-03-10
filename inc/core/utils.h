#ifndef UTILS_H
#define UTILS_H

#include "json.h"
namespace utils {
    using JsObj = nlohmann::json;
    using JsArry = nlohmann::json;

    JsObj string2Json(const std::string &strJson);

    bool isJsonArray(JsArry &json);
    int getJsonArraySize(JsArry &json);
    JsObj getJsonArray(JsArry &json, int pos);

    bool isJsonObject(JsObj &json);
    int getJsonValueInt(JsObj &json, const std::string &key, int defVal = 0);
    bool getJsonValueBool(JsObj &json, const std::string &key, bool defVal = false);
    std::string getJsonValueString(JsObj &json, const std::string &key, std::string defVal = "");

    std::string getFileName(const std::string& path);
    std::string getFilePath(const std::string& path);
    std::vector<std::string> splitPath(const std::string& path, char delimiter = '/');

    std::string exec(const char* cmd);
}

#endif // UTILS_H

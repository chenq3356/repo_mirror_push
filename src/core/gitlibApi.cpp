#include "gitlibApi.h"
#include "httpclient.h"
#include "utils.h"
namespace gitlibApi
{

std::string FLAGS_host = "https://gitlab.example.com";
std::string FLAGS_token = "";

//curl --header "Private-Token: <your_access_token>" "gitlab.example.com/api/v4/groups?id=<id>&search=<name>
int getGroupId(int parent_id, std::string name)
{
    http::Headers headers;
    headers.emplace("Private-Token", FLAGS_token);

    std::string path = "/api/v4/groups?id="+std::to_string(parent_id) +"&search=" + name;
    std::string url = FLAGS_host + path;

    http::Result result = http::Get(url, headers);
    if (CURLcode::CURLE_OK != result.code_) {
        printf("%s http get error(%d)\n", __FUNCTION__, (int)result.code_);
        return -2;
    }

    utils::JsObj resdata = utils::string2Json(result.body_);
    int size = utils::getJsonArraySize(resdata);
    for (int i = 0; i < size; i++)
    {
        utils::JsObj jsitem = utils::getJsonArray(resdata, i);
        std::string temp = utils::getJsonValueString(jsitem, "name", "");
        if (0 == name.compare(temp)) {
            return utils::getJsonValueInt(jsitem, "id", -1);
        }
    }
    return -1;
}

// {"name":groupName,"path":groupName,"parent_id":pgroupId}
int createGroup(int id, std::string name)
{
    utils::JsObj reqdata;
    reqdata["name"] = name;
    reqdata["path"] = name;
    reqdata["parent_id"] = id;

    http::Headers headers;
    headers.emplace("Private-Token", FLAGS_token);

    std::string path = "/api/v4/groups";
    std::string url = FLAGS_host + path;

    http::Result result = http::Post(url, reqdata.dump(), headers, "application/json");
    if (CURLcode::CURLE_OK != result.code_) {
        printf("%s http post error(%d)\n", __FUNCTION__, (int)result.code_);
        return -2;
    }

    utils::JsObj resdata = utils::string2Json(result.body_);
    return utils::getJsonValueInt(resdata, "id", -1);
}

bool isBranchExist(int project_id, std::string branch_name)
{
    if (project_id < 0) {
        return false;
    }

    http::Headers headers;
    headers.emplace("Private-Token", FLAGS_token);

    std::string path = "/api/v4/projects/" + std::to_string(project_id) + "/repository/branches";
    std::string url = FLAGS_host + path;

    http::Result result = http::Get(url, headers);
    if (CURLcode::CURLE_OK != result.code_) {
        printf("%s http get error(%d)\n", __FUNCTION__, (int)result.code_);
        return -2;
    }

    utils::JsObj resdata = utils::string2Json(result.body_);
    int size = utils::getJsonArraySize(resdata);
    for (int i = 0; i < size; i++)
    {
        utils::JsObj jsitem = utils::getJsonArray(resdata, i);
        std::string temp = utils::getJsonValueString(jsitem, "name", "");
        if (0 == branch_name.compare(temp)) {
            return true;
        }
    }
    return false;
}

std::string getProjectUrl(int parent_id, std::string name, bool* empty_repo, int* project_id)
{
    if (empty_repo) {
        *empty_repo = true;
    }

    if (project_id) {
        *empty_repo = -1;
    }

    http::Headers headers;
    headers.emplace("Private-Token", FLAGS_token);

    std::string path = "/api/v4/groups/" + std::to_string(parent_id) + "/projects?search=" + name;
    std::string url = FLAGS_host + path;

    http::Result result = http::Get(url, headers);
    if (CURLcode::CURLE_OK != result.code_) {
        printf("%s http get error(%d)\n", __FUNCTION__, (int)result.code_);
        return "";
    }

    utils::JsObj resdata = utils::string2Json(result.body_);
    int size = utils::getJsonArraySize(resdata);
    for (int i = 0; i < size; i++)
    {
        utils::JsObj jsitem = utils::getJsonArray(resdata, i);
        std::string temp = utils::getJsonValueString(jsitem, "name", "");
        if (0 == name.compare(temp)) {
            if (empty_repo) {
                *empty_repo = utils::getJsonValueBool(jsitem, "empty_repo", false);
            }
            if (project_id) {
                *project_id = utils::getJsonValueInt(jsitem, "id", -1);
            }
            return utils::getJsonValueString(jsitem, "ssh_url_to_repo", "");
        }
    }
    return "";
}

std::string createProject(int parent_id, std::string name)
{
    printf("createProject: %s\n", name.c_str());

    //{"name": proName,"description":proName,"path": proName,"namespace_id": str(currgroupId),"initialize_with_readme": "false"}
    utils::JsObj reqdata;
    reqdata["name"] = name;
    reqdata["path"] = name;
    reqdata["namespace_id"] = parent_id;
    reqdata["public_jobs"] = false;
    reqdata["initialize_with_readme"] = false;

    http::Headers headers;
    headers.emplace("Private-Token", FLAGS_token);

    std::string path = "/api/v4/projects";
    std::string url = FLAGS_host + path;

    http::Result result = http::Post(url, reqdata.dump(), headers, "application/json");
    if (CURLcode::CURLE_OK != result.code_) {
        printf("%s http post error(%d)\n", __FUNCTION__, (int)result.code_);
        return "";
    }

    utils::JsObj resdata = utils::string2Json(result.body_);
    return utils::getJsonValueString(resdata, "ssh_url_to_repo", "");
}

}

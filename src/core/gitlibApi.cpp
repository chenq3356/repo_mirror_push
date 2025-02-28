#include "gitlibApi.h"
#include "httplib.h"
#include "utils.h"

namespace gitlibApi
{

std::string FLAGS_host = "https://gitlab.example.com";
std::string FLAGS_token = "";


//curl --header "Private-Token: <your_access_token>" "gitlab.example.com/api/v4/groups?id=<id>&search=<name>
int getGroupId(int parent_id, std::string name)
{
    httplib::Headers header;
    header.emplace("Private-Token", FLAGS_token);

    std::string path = "/api/v4/groups?id="+std::to_string(parent_id) +"&search=" + name;

    httplib::Client cli(FLAGS_host);
    httplib::Result result = cli.Get(path, header);
    if (httplib::Error::Success != result.error()) {
        printf("%s http get error(%d)\n", __FUNCTION__, (int)result.error());
        return -2;
    }
    const httplib::Response &resp = result.value();
    utils::JsObj resdata = utils::string2Json(resp.body);
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

    httplib::Headers header;
    header.emplace("Private-Token", FLAGS_token);

    std::string path = "/api/v4/groups";

    httplib::Client cli(FLAGS_host);
    httplib::Result result = cli.Post(path, header, reqdata.dump(), "application/json");
    if (httplib::Error::Success != result.error()) {
        printf("%s http get error(%d)\n", __FUNCTION__, (int)result.error());
        return -2;
    }

    const httplib::Response &resp = result.value();
    utils::JsObj resdata = utils::string2Json(resp.body);
    return utils::getJsonValueInt(resdata, "id", -1);
}

bool isBranchExist(int project_id, std::string branch_name)
{
    if (project_id < 0) {
        return false;
    }

    httplib::Headers header;
    header.emplace("Private-Token", FLAGS_token);

    std::string path = "/api/v4/projects/" + std::to_string(project_id) + "/repository/branches";

    httplib::Client cli(FLAGS_host);
    httplib::Result result = cli.Get(path, header);
    const httplib::Response &resp = result.value();
    utils::JsObj resdata = utils::string2Json(resp.body);
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

    httplib::Headers header;
    header.emplace("Private-Token", FLAGS_token);

    std::string path = "/api/v4/groups/" + std::to_string(parent_id) + "/projects?search=" + name;

    httplib::Client cli(FLAGS_host);
    httplib::Result result = cli.Get(path, header);
    if (httplib::Error::Success != result.error()) {
        printf("%s http get error(%d)\n", __FUNCTION__, (int)result.error());
        return "";
    }

    const httplib::Response &resp = result.value();
    utils::JsObj resdata = utils::string2Json(resp.body);
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

    httplib::Headers header;
    header.emplace("Private-Token", FLAGS_token);

    std::string path = "/api/v4/projects";

    httplib::Client cli(FLAGS_host);
    httplib::Result result = cli.Post(path, header, reqdata.dump(), "application/json");
    if (httplib::Error::Success != result.error()) {
        printf("%s http get error(%d)\n", __FUNCTION__, (int)result.error());
        return "";
    }

    const httplib::Response &resp = result.value();
    utils::JsObj resdata = utils::string2Json(resp.body);

    return utils::getJsonValueString(resdata, "ssh_url_to_repo", "");
}

}

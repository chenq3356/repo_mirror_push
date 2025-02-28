#ifndef GITLIBAPI_H
#define GITLIBAPI_H

#include <string>


// 官方文档 https://docs.gitlab.com/api/
// Gitlab的Api的文档入口为http://your_ip/help/api/README.md
// Gitlab的Api的文档入口为http://your_ip/help/api/api_resources.md
namespace gitlibApi
{

extern std::string FLAGS_host;
extern std::string FLAGS_token;

int getGroupId(int parent_id, std::string name);

int createGroup(int parent_id, std::string name);

bool isBranchExist(int project_id, std::string branch_name);

std::string getProjectUrl(int parent_id, std::string name, bool* empty_repo = nullptr, int* project_id = nullptr);

std::string createProject(int parent_id, std::string name);

}

#endif // GITLIBAPI_H

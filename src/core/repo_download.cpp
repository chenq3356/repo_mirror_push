#include "repo_download.h"
#include "config.h"
#include "gitlibApi.h"
#include "xmlReader.h"
#include "utils.h"

namespace repo {
namespace download {

static config::RepoDownloadCfg s_downloadCfg;
bool execGitPull(std::string path, std::string giturl, std::string branch)
{
    // 脚本路径和参数
    printf("begin upload...\n");
    std::string scriptPath = "./git_pull.sh";    // 脚本路径
    std::string arg1 = path;                    // 第一个参数: 本地路径
    std::string arg2 = giturl;                  // 第二个参数: 远程URL
    std::string arg3 = branch; // 第三个参数: 远程分支名

    // 构建完整的命令
    std::string command = scriptPath + " " + arg1 + " " + arg2 + " " + arg3;
    try {
        std::string result = utils::exec(command.c_str());
        if (std::string::npos != result.find("error") || std::string::npos != result.find("fatal")) {
            printf("ERROR: %s\n", result.c_str());
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        printf("ERROR: %s\n", command.c_str());
        return false;
    }
}

bool execMirrorPull(std::string path, std::string giturl)
{
    // 脚本路径和参数
    printf("begin pull...\n");
    std::string scriptPath = "./mirror_pull.sh";    // 脚本路径
    std::string arg1 = path;                    // 第一个参数: 本地路径
    std::string arg2 = giturl;                  // 第二个参数: 远程URL

    // 构建完整的命令
    std::string command = scriptPath + " " + arg1 + " " + arg2;
    try {
        std::string result = utils::exec(command.c_str());
        if (std::string::npos != result.find("error") || std::string::npos != result.find("fatal")) {
            printf("ERROR: %s\n", result.c_str());
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        printf("ERROR: %s\n", command.c_str());
        return false;
    }
}

void repo_download(manifest::XMLReader* reader)
{
    if (!config::getRepoDownloadConfig(s_downloadCfg)) {
        return;
    }
    if (!s_downloadCfg.enable_) {
        return;
    }

    std::vector<tinyxml2::XMLElement*> uploadList = reader->getProjectList();
    int uploadSize = uploadList.size();
    printf("start repo_download...%d\n", uploadSize);

    for (int i = 0; i < uploadSize; i++) {
        tinyxml2::XMLElement* element = uploadList[i];

        std::string name = reader->getName(element);
        printf("[%d/%d] %s: \n", i+1, uploadSize, name.c_str());

        std::string giturl = reader->getNameFillFetch(element) + std::string(".git");
        std::string fullpath = s_downloadCfg.projectsPath_
                + std::string("/")
                + reader->getPath(element);
        if (s_downloadCfg.mirror_) {
            if (!execMirrorPull(fullpath, giturl)) {
                printf("execMirrorPull %s failed\n", name.c_str());
                exit(0);
            }
        } else {
            std::string branch = reader->getBranchName(element);
            if (!execGitPull(fullpath, giturl, branch)) {
                printf("execGitPull %s failed\n", name.c_str());
                exit(0);
            }
        }
    }
    printf("repo_download end\n");
}


} // namespace download
} // repo

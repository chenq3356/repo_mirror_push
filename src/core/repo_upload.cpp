#include "repo_upload.h"

#include "config.h"
#include "xmlReader.h"
#include "gitlibApi.h"
#include "utils.h"

namespace repo {

namespace upload {

static config::RepoUploadCfg s_uploadCfg;

bool execGitPush(std::string path, std::string giturl, std::string branch)
{
    // 脚本路径和参数
    printf("begin upload...\n");
    std::string scriptPath = "./git_push.sh";    // 脚本路径
    std::string arg1 = path;                    // 第一个参数: 本地路径
    std::string arg2 = giturl;                  // 第二个参数: 远程URL
    std::string arg3 = branch; // 第三个参数: 远程分支名
    std::string arg4 = s_uploadCfg.to_unshallow_ ? "true" : "false";

    // 构建完整的命令
    std::string command = scriptPath + " " + arg1 + " " + arg2 + " " + arg3 + " " + arg4;
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

bool execMirrorPush(std::string path, std::string giturl)
{
    // 脚本路径和参数
    printf("begin upload...\n");
    std::string scriptPath = "./mirror_push.sh";    // 脚本路径
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

bool pushAProject(std::string path, std::string giturl, std::string branch)
{
    if (s_uploadCfg.mirror_) {
        std::string fullpath = s_uploadCfg.projectsPath_
                + std::string("/")
                + path
                + std::string(".git");
        if (!execMirrorPush(fullpath, giturl)) {
            return false;
        }
    } else {
        std::string fullpath = s_uploadCfg.projectsPath_
                + std::string("/")
                + path;
        if (!execGitPush(fullpath, giturl, branch)) {
            return false;
        }
    }
    return true;
}


std::string generateAProject(int parent_id, std::string path, std::string branch, bool* isMustPush) {
    if (isMustPush) {
        *isMustPush = true;
    }

    std::string sshUrl;
    bool empty_repo = true;
    std::string projectName = utils::getFileName(path);
    if (s_uploadCfg.repeate_) {
        sshUrl = gitlibApi::getProjectUrl(parent_id, projectName, &empty_repo);
        if (sshUrl.empty()) {
            sshUrl = gitlibApi::createProject(parent_id, projectName);
        }
    } else {
        int project_id  = -1;
        sshUrl = gitlibApi::getProjectUrl(parent_id, projectName, &empty_repo, &project_id);
        if (isMustPush && !sshUrl.empty() && project_id > 0) {
            if (gitlibApi::isBranchExist(project_id, branch)) {
                *isMustPush = false;
            }
        }
        if (sshUrl.empty()) {
            sshUrl = gitlibApi::createProject(parent_id, projectName);
        }
    }

    if (s_uploadCfg.mirror_ && !empty_repo && isMustPush) {
        *isMustPush = false;
    }
    return sshUrl;
}

int generateAGroup(TreeNode* root, std::string path)
{
    std::vector<std::string> dirList = utils::splitPath(path);

    // 保留目录部分
    dirList.pop_back();

    TreeNode* current = root;
    for (const auto& part : dirList) {
        auto iter = current->children_.find(part);
        if (iter != current->children_.end()) {
            current = iter->second;
        } else {
            int id = gitlibApi::getGroupId(current->id_, part);
            if (id < 0) {
                id = gitlibApi::createGroup(current->id_, part);
            }

            // 取不到group id，返回负数
            if (id < 0) {
                printf("get group id error\n");
                return -1;
            }

            TreeNode* temp = new TreeNode(id, part);
            current->children_.emplace(part, temp);
            current = temp;
        }
    }
    return current->id_;
}

void repo_upload(manifest::XMLReader* reader)
{
    if (!config::getRepoUploadConfig(s_uploadCfg)) {
        return;
    }
    if (!s_uploadCfg.enable_) {
        return;
    }

    printf("start repo_upload...\n");

    gitlibApi::FLAGS_host = s_uploadCfg.targetHost_;
    gitlibApi::FLAGS_token = s_uploadCfg.targetToken_;

    int rootId = s_uploadCfg.targetGroupId_;
    std::string manifestsPath = s_uploadCfg.projectsPath_ + "/.repo/manifests";
    std::string repoPath = s_uploadCfg.projectsPath_ + "/.repo/repo";

    std::vector<tinyxml2::XMLElement*> uploadList = reader->getProjectList();
    int uploadSize = uploadList.size();
    TreeNode* root = new TreeNode(rootId, "root");
    for (int i = 0; i < uploadSize; i++) {
        tinyxml2::XMLElement* element = uploadList[i];

        std::string name = reader->getName(element);
        printf("[%d/%d] %s: \n", i+1, uploadSize, name.c_str());

        // 递归创建group
        int parent_id = generateAGroup(root, reader->getNameFillRemote(element));
        if (parent_id < 0) {
            printf("generateGroup %s failed\n", name.c_str());
            exit(0);
        }

        // 创建project
        bool isMustPush = true;
        std::string branch = reader->getBranchName(element);
        std::string sshurl = generateAProject(parent_id, name, branch, &isMustPush);
        if (sshurl.empty()) {
            printf("generateAProject %s failed\n", name.c_str());
            exit(0);
        }

        // 提交project
        if (isMustPush) {
            if (!pushAProject(reader->getPath(element), sshurl, branch)) {
                printf("execPush %s failed\n", name.c_str());
                exit(0);
            }
        }
    }
    printf("repo_upload end\n");

    // 提交manifests
    if (!manifestsPath.empty()) {
        printf("begin upload manifests...\n");
        bool isMustPush = true;
        std::string sshurl = generateAProject(rootId, "manifests", "master", &isMustPush);
        if (!sshurl.empty()) {
            execGitPush(manifestsPath, sshurl, "master");
        }
        printf("begin upload manifests end\n");
    }

    // 提交repo
    if (!repoPath.empty()) {
        printf("begin upload repo...\n");
        bool isMustPush = true;
        std::string sshurl = generateAProject(rootId, "repo", "master", &isMustPush);
        if (!sshurl.empty()) {
            execGitPush(repoPath, sshurl, "master");
        }
        printf("begin upload repo end\n");
    }
    //printTree(root);
    delete root;
}

} // namespace upload
} // namespace repo


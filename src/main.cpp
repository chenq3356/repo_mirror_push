#include "INIReader.h"
#include "xmlReader.h"
#include "gitlibApi.h"
#include "utils.h"

bool s_lossRemote   = false;    // 旧的远程是否已丢失
bool s_upload       = false;    //是否上传代码
bool s_commitAgain  = false;    //是否重复提交，否的话跳过已经提交过的仓库
std::string s_basePath;         //本地代码的路径
std::string s_projectUrl;       //目标gitlab地址
std::string s_apiToken;         //目标gitlab授权的API个人访问令牌
std::string s_namespaceName;    //目标库名称
int s_namespaceId   = -1;       //目标库分组id（将按源目录自动提交到此分组下）
std::string s_manifestsPath;    //本地要提交的manifests仓库路径
std::string s_repoPath;         //本地要提交的repo仓库路径

std::string s_xmlOutFile;     //将指定的XML转换成单文件的形式，为空不输出
std::string s_xmlBasePath;    //XML文件的路径
std::string s_xmlBaseFile;    //预处理的XML文件

// 定义目录树的节点结构体
struct TreeNode {
    int id_;            // group id
    std::string name_;  // group name
    std::map<std::string, TreeNode*> children_;

    TreeNode(int id, const std::string& name) : id_(id), name_(name) {}

    ~TreeNode() {
        for (auto& child : children_) {
            delete child.second;
        }
    }
};

bool parseParameter()
{
    INIReader reader("setting.ini");
    if (reader.ParseError() < 0) {
        return false;
    }
    s_upload        = reader.GetBoolean("UploadInfo", "upload", false);
    s_lossRemote    = reader.GetBoolean("UploadInfo", "lossRemote", false);
    s_commitAgain   = reader.GetBoolean("UploadInfo", "commitAgain", false);
    s_basePath      = reader.GetString("UploadInfo", "basePath", "");
    s_manifestsPath = reader.GetString("UploadInfo", "manifestsPath", "");
    s_repoPath      = reader.GetString("UploadInfo", "repoPath", "");
    s_projectUrl    = reader.GetString("UploadInfo", "projectUrl", "");
    s_apiToken      = reader.GetString("UploadInfo", "apiToken", "");
    s_namespaceName = reader.GetString("UploadInfo", "namespaceName", "");
    s_namespaceId   = reader.GetInteger("UploadInfo", "namespaceId", -1);

    s_xmlOutFile    = reader.GetString("XMLInfo", "outFile", "");
    s_xmlBasePath   = reader.GetString("XMLInfo", "basePath", "");
    s_xmlBaseFile   = reader.GetString("XMLInfo", "baseFile", "");

    // 删除所有末尾的'/'
    while (!s_basePath.empty() && s_basePath.back() == '/') {
        s_basePath.pop_back();
    }
    while (!s_xmlBasePath.empty() && s_xmlBasePath.back() == '/') {
        s_xmlBasePath.pop_back();
    }
    return true;
}

bool checkParameter() {
    if (s_upload) {
        if (s_basePath.empty()) {
            return false;
        }
        if (s_projectUrl.empty()) {
            return false;
        }
        if (s_apiToken.empty()) {
            return false;
        }
        if (s_namespaceId < 0) {
            return false;
        }
    }
    if (s_xmlBasePath.empty()) {
        return false;
    }
    if (s_xmlBaseFile.empty()) {
        return false;
    }
    return true;
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

bool execPush(std::string path, std::string giturl, std::string branch)
{
    // 脚本路径和参数
    printf("begin upload...\n");
    std::string scriptPath = "./mirror_push.sh";    // 脚本路径
    std::string arg1 = path;                    // 第一个参数: 本地路径
    std::string arg2 = giturl;                  // 第二个参数: 远程URL
    std::string arg3 = branch; // 第三个参数: 远程分支名
    std::string arg4 = s_lossRemote ? "true" : "false";

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

bool pushAProject(int parent_id, ProjectInfo& info)
{
    std::string projectName = utils::getFileName(info.name_);

    int project_id = -1;
    bool empty_repo = true;
    std::string sshUrl = gitlibApi::getProjectUrl(parent_id, projectName, &empty_repo, &project_id);
    if (!empty_repo && project_id >= 0) {
        if (gitlibApi::isBranchExist(project_id, info.branch_)) {
            empty_repo = false;
        } else {
            empty_repo = true;
        }
    }

    if (sshUrl.empty()) {
        empty_repo = true;
        sshUrl = gitlibApi::createProject(parent_id, projectName);
    }

    // 取不到远程地址，返回失败
    if (sshUrl.empty()) {
        printf("get ssh url error\n");
        return false;
    }

    // 不为空，已经提交过了
    if (!s_commitAgain && !empty_repo) {
        return true;
    }

    if (info.isFullPath) {
        return execPush(info.path_, sshUrl, info.branch_);
    } else {
        std::string fullpath = s_basePath + std::string("/") + info.path_;
        return execPush(fullpath, sshUrl, info.branch_);
    }
}

void repo_upload(XMLReader* reader)
{
    printf("start repo_upload\n");
    gitlibApi::FLAGS_host   = s_projectUrl;
    gitlibApi::FLAGS_token  = s_apiToken;

    std::vector<ProjectInfo> uploadList = reader->getProjects();
    if (!s_manifestsPath.empty()) {
        ProjectInfo info;
        info.isFullPath = true;
        info.name_      = "manifests";
        info.path_      = s_manifestsPath;
        info.branch_    = "master";
        uploadList.push_back(info);
    }
    if (!s_repoPath.empty()) {
        ProjectInfo info;
        info.isFullPath = true;
        info.name_      = "repo";
        info.path_      = s_repoPath;
        info.branch_    = "master";
        uploadList.push_back(info);
    }

    int uploadSize = uploadList.size();
    TreeNode* root = new TreeNode(s_namespaceId, s_namespaceName);
    for (int i = 0; i < uploadSize; i++) {

        printf("[%d/%d] %s: \n", i+1, uploadSize, uploadList[i].name_.c_str());

        int parent_id = generateAGroup(root, uploadList[i].name_);
        if (parent_id < 0) {
            printf("generateGroup %s failed\n", uploadList[i].name_.c_str());
            exit(0);
        }

        if (!pushAProject(parent_id, uploadList[i])) {
            printf("pushAProject %s failed\n", uploadList[i].name_.c_str());
            exit(0);
        }
    }

    printf("repo_upload end\n");
    //printTree(root);
    delete root;
}

void revisionTest()
{
    std::vector<std::string> revisionList;
    revisionList.push_back("main");
    revisionList.push_back("a56e0e17e23f925ff44c75e5b89330ccc2598640");
    revisionList.push_back("v1.0.0");
    revisionList.push_back("1.0.0");
    revisionList.push_back("HEAD~1");
    revisionList.push_back("refs/heads/master");
    revisionList.push_back("refs/tags/tag");
    revisionList.push_back("refs/changes/changes");

    for (auto revision : revisionList)
    {
        XMLReader::isBranchName(revision, true);
    }
}

int main() {
    if (!parseParameter()) {
        printf ("parseParameter error!\n");
        exit(0);
    }

    if (!checkParameter()) {
        printf ("checkParameter error!\n");
        exit(0);
    }

    //revisionTest();

    XMLReader reader(s_xmlBasePath, s_xmlBaseFile);

    if (s_upload) {
        repo_upload(&reader);
    }

    if (!s_xmlOutFile.empty()) {
        reader.saveAsXML(s_xmlOutFile, s_lossRemote);
    }
    return 0;
}

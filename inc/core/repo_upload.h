#ifndef REPO_UPLOAD_H
#define REPO_UPLOAD_H

#include <map>
#include "xmlReader.h"

namespace repo {

namespace upload {

// 定义目录树的节点结构体, 用于保存groupId
struct TreeNode {
    int id_;            // groupId
    std::string name_;  // groupName
    std::map<std::string, TreeNode*> children_;

    TreeNode(int id, const std::string& name) : id_(id), name_(name) {}

    ~TreeNode() {
        for (auto& child : children_) {
            delete child.second;
        }
    }
};

bool execGitPush(std::string path, std::string giturl, std::string branch);

bool execMirrorPush(std::string path, std::string giturl);

std::string generateAProject(int parent_id, std::string path, std::string branch, bool* isMustPush);

int generateAGroup(TreeNode* root, std::string path);

void repo_upload(manifest::XMLReader* reader);

} // namespace upload
} // namespace repo


#endif // REPO_UPLOAD_H

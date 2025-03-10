#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace config {

struct RepoDownloadCfg{
    bool enable_;                   // 是否执行
    bool mirror_;                   // 是否以镜像的方式下载
    std::string projectsPath_;      // 下载后存放的本地根路径
};

struct RepoUploadCfg{
    bool enable_;               // 是否执行
    bool mirror_;               // 是否以镜像的方式提交，本地的源码也需要是镜像拉取
    bool repeate_;              // 允许重复提交操作
    bool to_unshallow_;         // 是否将浅克隆转为完整克隆，否的话则重新创建git
    std::string projectsPath_;  // 本地代码的根路径
    std::string targetHost_;    // 目标gitlab地址,域名
    std::string targetToken_;   // 目标gitlab授权的API个人访问令牌
    int targetGroupId_;         // 目标远程仓库的根路径的id
};

struct xmlCfg{
    std::string outFile_;
    std::string basePath_;
    std::string baseFile_;
};

bool getRepoDownloadConfig(RepoDownloadCfg &cfg);
bool getRepoUploadConfig(RepoUploadCfg& cfg);
bool getXMLConfig(xmlCfg &cfg);

} // namespace config

#endif // CONFIG_H

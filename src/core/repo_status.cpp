#include "repo_status.h"
#include "config.h"
#include "utils.h"

namespace repo {

namespace status {
static config::RepoStatusCfg s_statusCfg;
void repo_status(manifest::XMLReader* reader)
{
    if (!config::getRepoStatusConfig(s_statusCfg)) {
        return;
    }
    if (!s_statusCfg.enable_) {
        return;
    }

    std::vector<tinyxml2::XMLElement*> uploadList = reader->getProjectList();
    int uploadSize = uploadList.size();
    printf("start repo_status...%d\n", uploadSize);
    for (int i = 0; i < uploadSize; i++) {
        tinyxml2::XMLElement* element = uploadList[i];

        std::string name = reader->getName(element);
        printf("[%d/%d] %s: \n", i+1, uploadSize, name.c_str());

        std::string path = s_statusCfg.projectsPath_ + "/" + reader->getPath(element);
        std::string revision = reader->getRevision(element);
        std::string branch = reader->getBranchName(element);

        std::string command = "./git_status.sh " + path + " " + revision + " " + branch;
        try {
            std::string result = utils::exec(command.c_str());
            if (std::string::npos != result.find("error") || std::string::npos != result.find("fatal")) {
                printf("ERROR: %s\n", result.c_str());
                exit(0);
            }
            printf("%s\n", result.c_str());
        } catch (const std::exception& e) {
            printf("ERROR: %s\n", command.c_str());
            exit(0);
        }
    }
    printf("repo_status end\n");
}

} // namespace status
} // namespace repo


#include "repo_upload.h"
#include "repo_download.h"
#include "config.h"

static config::xmlCfg sxmlCfg;

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
        manifest::detail::isBranchName(revision, true);
    }
}

int main() {
    if (!config::getXMLConfig(sxmlCfg)) {
        return 0;
    }

    manifest::XMLReader reader(sxmlCfg.basePath_, sxmlCfg.baseFile_);

    repo::download::repo_download(&reader);
    repo::upload::repo_upload(&reader);

    if (!sxmlCfg.outFile_.empty()) {
        reader.saveAsXML(sxmlCfg.outFile_);
    }

    return 0;
}

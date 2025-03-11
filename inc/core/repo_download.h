#ifndef REPO_DOWNLOAD_H
#define REPO_DOWNLOAD_H
#include <map>
#include "xmlReader.h"

namespace repo {
namespace download {
    bool execGitPull(std::string path, std::string giturl, std::string branch);
    bool execMirrorPull(std::string path, std::string giturl);

    void repo_download(manifest::XMLReader* reader);


} // namespace download
} // repo


#endif // REPO_DOWNLOAD_H

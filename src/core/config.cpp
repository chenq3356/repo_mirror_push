#include "config.h"
#include "INIReader.h"

namespace config {

bool getRepoDownloadConfig(RepoDownloadCfg &cfg) {
    INIReader reader("setting.ini");
    if (reader.ParseError() < 0) {
        return false;
    }
    cfg.enable_         = reader.GetBoolean("RepoDownload", "enable", false);
    cfg.mirror_         = reader.GetBoolean("RepoDownload", "mirror", false);
    cfg.projectsPath_   = reader.GetString("RepoDownload", "projectsPath", "");
    return true;
}

bool getRepoUploadConfig(RepoUploadCfg& cfg) {
    INIReader reader("setting.ini");
    if (reader.ParseError() < 0) {
        return false;
    }

    cfg.enable_         = reader.GetBoolean("RepoUpload", "enable", false);
    cfg.mirror_         = reader.GetBoolean("RepoUpload", "mirror", false);
    cfg.repeate_        = reader.GetBoolean("RepoUpload", "repeate", false);
    cfg.to_unshallow_   = reader.GetBoolean("RepoUpload", "to_unshallow", false);
    cfg.projectsPath_   = reader.GetString("RepoUpload", "projectsPath", "");
    cfg.targetHost_       = reader.GetString("RepoUpload", "targetHost", "");
    cfg.targetToken_      = reader.GetString("RepoUpload", "targetToken", "");
    cfg.targetGroupId_    = reader.GetInteger("RepoUpload", "targetGroupId", -1);
    return true;
}

bool getRepoStatusConfig(RepoStatusCfg& cfg)
{
    INIReader reader("setting.ini");
    if (reader.ParseError() < 0) {
        return false;
    }
    cfg.enable_         = reader.GetBoolean("RepoStatus", "enable", false);
    cfg.projectsPath_   = reader.GetString("RepoStatus", "projectsPath", "");
    return true;
}

bool getXMLConfig(xmlCfg &cfg) {
    INIReader reader("setting.ini");
    if (reader.ParseError() < 0) {
        return false;
    }
    cfg.outFile_    = reader.GetString("XMLConfig", "outFile", "");
    cfg.basePath_   = reader.GetString("XMLConfig", "basePath", "");
    cfg.baseFile_   = reader.GetString("XMLConfig", "baseFile", "");
    return true;
}

} //namespace config

#pragma once

#include <tesla.hpp>
#include <filesystem>

#define VERSION "1.0.1"

#define LOGFILE "/config/Cling-Wrap/overlayLog.txt"

namespace nxfs = std::filesystem;

enum status {
    unwrapped = 0,
    wrapped = 1,
    bothPresent = 2,
    neitherPresent = 3
};

struct directory {
    std::string dirName;
    tsl::elm::ListItem *listItem;
    nxfs::path dirPath;
    nxfs::path altPath;

    std::string getName() {
        return dirName;
    }
    const std::string getPath() {
        return dirPath;
    }
    std::string getAltPath() {
        return altPath;
    }
};

class GuiMain : public tsl::Gui {
  private:
    FsFileSystem m_fs;
    status tinfoilReady;
    tsl::elm::ListItem *tinfoilReadyItem;
    std::list<directory> directoryListItems;
    bool scanned;

  public:
    GuiMain();
    ~GuiMain();

    virtual tsl::elm::Element *createUI();
    virtual void update() override;

  private:
    void updateStatus(const directory &dir);
    status getStatus(const directory &dir);

    bool FS_DirExists(FsFileSystem *fs, const char *path);
    Result FS_RenameDir(FsFileSystem *fs, const char *old_dirname, const char *new_dirname);

    void rename(const directory &dir);
    void renameAll(status s);
};
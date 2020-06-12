#pragma once

#include <tesla.hpp>
#include <filesystem>

#define VERSION "1.0.0"

#define LOGFILE "/config/Cling-Wrap/overlayLog.txt"

#define BOOTLOADERPATH "/bootloader"
#define ALTBOOTLOADERPATH "/_bootloader"

#define KIPSPATH "/atmosphere/kips"   
#define ALTKIPSPATH "/atmosphere/_kips"

namespace nxfs = std::filesystem;

enum status {
    notReady = 0,
    ready = 1,
    error = 2
};

struct directory {
    std::string dirName;
    tsl::elm::ListItem *listItem;
    status dirStatus;
    nxfs::path dirPath;
    nxfs::path altPath;

    void setStatus(status s) {
        dirStatus = s;
    }

    std::string getName() {
        return dirName;
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
    bool FS_DirExists(FsFileSystem *fs, const char *path);
    Result FS_RenameDir(FsFileSystem *fs, const char *old_dirname, const char *new_dirname);
    std::string getText();
};
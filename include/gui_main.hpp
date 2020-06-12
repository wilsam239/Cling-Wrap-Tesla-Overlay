#pragma once

#include <tesla.hpp>

#define VERSION "1.0.0"

#define LOGFILE "/config/Cling-Wrap/overlayLog.txt"

#define BOOTLOADERPATH "/bootloader"
#define ALTBOOTLOADERPATH "/_bootloader"

#define KIPSPATH "/atmosphere/kips"   
#define ALTKIPSPATH "/atmosphere/_kips"

enum status {
    ready = 1,
    notReady = 0,
    error = -1
};

class GuiMain : public tsl::Gui {
  private:
    FsFileSystem m_fs;
    status tinfoilReady;
    tsl::elm::ListItem *tinfoilReadyItem;

  public:
    GuiMain();
    ~GuiMain();

    virtual tsl::elm::Element *createUI();
    virtual void update() override;

  private:
    void updateStatus();
    bool FS_DirExists(FsFileSystem *fs, const char *path);
    Result FS_RenameDir(FsFileSystem *fs, const char *old_dirname, const char *new_dirname);
    std::string getText();
};
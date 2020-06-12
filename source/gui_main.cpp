#include "gui_main.hpp"
#include <tesla.hpp>    // The Tesla Header
#include <sstream>
#include <fstream>

constexpr const char *const descriptions[3] = {
    [0] = "Unwrapped | \uE098",
    [1] = "Wrapped | \uE098",
    [2] = "Error | \uE0F4",
    
};

GuiMain::GuiMain() {
    Result rc = fsOpenSdCardFileSystem(&this->m_fs);
    if (R_FAILED(rc))
        return;

    directory bootloader = {
        .dirName = "Bootloader",
        .listItem = new tsl::elm::ListItem(bootloader.dirName),
        .dirPath = BOOTLOADERPATH,
        .altPath = ALTBOOTLOADERPATH
    };

    bootloader.listItem->setClickListener([this, bootloader](u64 keys) -> bool {
        if(keys & KEY_A) {
            rename(bootloader);
            return true;
        }
        return false;
    });
    this->directoryListItems.push_back(std::move(bootloader));
    this->scanned = true;
}

GuiMain::~GuiMain() { 
    fsFsClose(&this->m_fs);
}

// Called when this Gui gets loaded to create the UI
// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
tsl::elm::Element *GuiMain::createUI() {
    //writeToLog("Entered create UI function");

    // A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
    // If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
    tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame("Cling Wrap", VERSION);

    if(this->directoryListItems.size() == 0) {
        const char *description = this->scanned ? "No conflicting folders found!" : "Scan failed!";

        auto *warning = new tsl::elm::CustomDrawer([description](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("\uE150", false, 180, 250, 90, renderer->a(0xFFFF));
            renderer->drawString(description, false, 110, 340, 25, renderer->a(0xFFFF));
        });

        rootFrame->setContent(warning);
    } else {
        tsl::elm::List *dirList = new tsl::elm::List();

        auto *readyAllButton = new tsl::elm::ListItem("Wrap");
        readyAllButton->setClickListener([this](u64 keys) -> bool { 
            if(keys & KEY_A) {
                renameAll(status::wrapped);
                return true;
            }
            return false;
        });
        dirList->addItem(readyAllButton);

        auto *resetAllButton = new tsl::elm::ListItem("Unwrap");
        resetAllButton->setClickListener([this](u64 keys) -> bool { 
            if(keys & KEY_A) {
                renameAll(status::unwrapped);
                return true;
            }
            return false;
        });
        dirList->addItem(resetAllButton);

        dirList->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("\uE016  These directories can be renamed individually.", false, x + 5, y + 20, 15, renderer->a(tsl::style::color::ColorDescription));
        }), 30);

        for (const auto &dir : this->directoryListItems) {
            dirList->addItem(dir.listItem);
        }

        // Add the list to the frame for it to be drawn
        rootFrame->setContent(dirList);
    }

    // Return the frame to have it become the top level element of this Gui
    return rootFrame;
}

// Called once every frame to update values
void GuiMain::update() {
    static u32 counter = 0;

    if (counter++ % 20 != 0) 
        return;
    
    for (const auto &dir : this->directoryListItems) {
        this->updateStatus(dir);
    }
}

void GuiMain::updateStatus(const directory &dir) {

    const char *desc = descriptions[this->getStatus(dir)];
    dir.listItem->setValue(desc);
}

status GuiMain::getStatus(const directory &dir) {
    const char *path = dir.dirPath.c_str();
    const char *alt = dir.altPath.c_str();

    if(this->FS_DirExists(&this->m_fs, path)) {
        return status::unwrapped;
    } else if(this->FS_DirExists(&this->m_fs, alt)) {
        return status::wrapped;
    } else {
        return status::error;
    }
}

/**
 * FS_DirExists
 * Checks if the given path exists as a directory
**/
bool GuiMain::FS_DirExists(FsFileSystem *fs, const char *path) {
	FsDir dir;
    //writeToLog("Initialised dir");

	char temp_path[FS_MAX_PATH];
    //writeToLog("Initialised temp_path");
	snprintf(temp_path, FS_MAX_PATH, path);
    //writeToLog("Wrote to temp_path");

	if (R_SUCCEEDED(fsFsOpenDirectory(fs, temp_path, FsDirOpenMode_ReadDirs, &dir))) {
		fsDirClose(&dir);
		return true;
	}

	return false;
}

/**
 * FS_RenameDir
 * Renames the old_dirname to new_dirname
**/
Result GuiMain::FS_RenameDir(FsFileSystem *fs, const char *old_dirname, const char *new_dirname) {
	Result ret = 0;

	char temp_path_old[FS_MAX_PATH], temp_path_new[FS_MAX_PATH];
	snprintf(temp_path_old, FS_MAX_PATH, old_dirname);
	snprintf(temp_path_new, FS_MAX_PATH, new_dirname);

	if (R_FAILED(ret = fsFsRenameDirectory(fs, temp_path_old, temp_path_new))) {
        std::stringstream err;
        err << "fsFsRenameDirectory(" << temp_path_old << ", " << temp_path_new << ") failed: 0x" << ret;
		//writeToLog(err.str());
        //std::cout << "Renaming diretories has failed." << std::endl;
		return ret;
	}

	return 0;
}


void GuiMain::rename(const directory &dir) {
    const char *path = dir.dirPath.c_str();
    const char *alt = dir.altPath.c_str();

    if(this->FS_DirExists(&this->m_fs, path)) {
        if(FS_RenameDir(&this->m_fs, path, alt) == 0) {
            // Success
        }
    } else if(this->FS_DirExists(&this->m_fs, alt)) {
        if(FS_RenameDir(&this->m_fs, alt, path) == 0) {
            // Success
        }
    }
}

void GuiMain::renameAll(status s) {
    //Rename all folders so that they are in the s status
    //wrapped = alt path present
    //unwrapped = path present
    for (const auto &dir : this->directoryListItems) {
        const char *path = dir.dirPath.c_str();
        const char *alt = dir.altPath.c_str();
        if(s == status::wrapped) {
            if(this->FS_DirExists(&this->m_fs, path)) {
                if(FS_RenameDir(&this->m_fs, path, alt) == 0) {
                    // Success
                }
            }
        } else if (s == status::unwrapped) {
            if(this->FS_DirExists(&this->m_fs, alt)) {
                if(FS_RenameDir(&this->m_fs, alt, path) == 0) {
                    // Success
                }
            }
        }
    }
}

#include "gui_main.hpp"
#include <tesla.hpp>    // The Tesla Header
#include <sstream>
#include <fstream>

constexpr const char *const descriptions[3] = {
    [0] = "Not Ready | \uE098",
    [1] = "Ready | \uE098",
    [2] = "Error | \uE0F4",
    
};

GuiMain::GuiMain() {
    Result rc = fsOpenSdCardFileSystem(&this->m_fs);
    if (R_FAILED(rc))
        return;

    directory bootloader = {
        .dirName = "Bootloader",
        .listItem = new tsl::elm::ListItem(bootloader.dirName),
        .dirStatus = status::notReady,
        .dirPath = BOOTLOADERPATH,
        .altPath = ALTBOOTLOADERPATH
    };

    bootloader.listItem->setClickListener([this, &bootloader](u64 keys) -> bool {
        if(keys & KEY_A) {
            if(bootloader.dirStatus == status::ready) {
                nxfs::rename(bootloader.altPath, bootloader.dirPath);
                bootloader.setStatus(status::notReady);
            } else if(bootloader.dirStatus == status::notReady) {
                nxfs::rename(bootloader.dirPath, bootloader.altPath);
                bootloader.setStatus(status::ready);
            }
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

/*void writeToLog(std::string msg) {
    std::ofstream logFile;
    logFile.open(LOGFILE, std::ofstream::out | std::ofstream::app);

    const auto p1 = std::chrono::system_clock::now();
    std::time_t today_time = std::chrono::system_clock::to_time_t(p1);

    if(logFile.is_open()) {
        logFile << msg << " - " << std::ctime(&today_time);
    }
    logFile.close();
}*/

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

        auto *readyAllButton = new tsl::elm::ListItem("Ready All");
        readyAllButton->setClickListener([this](u64 keys) -> bool { 
            if(keys & KEY_A) {
                for (auto &dir : this->directoryListItems) {
                    if(dir.dirStatus == status::notReady) {
                        nxfs::rename(dir.dirPath, dir.altPath);
                        dir.setStatus(status::ready);
                    }
                }
                return true;
            }
            return false;
        });
        dirList->addItem(readyAllButton);

        auto *unreadyAllButton = new tsl::elm::ListItem("Unready All");
        unreadyAllButton->setClickListener([this](u64 keys) -> bool { 
            if(keys & KEY_A) {
                for (auto &dir : this->directoryListItems) {
                    if(dir.dirStatus == status::ready) {
                        nxfs::rename(dir.altPath, dir.dirPath);
                        dir.setStatus(status::notReady);
                    }
                }
                return true;
            }
            return false;
        });
        dirList->addItem(unreadyAllButton);

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

    const char *desc = descriptions[dir.dirStatus];
    dir.listItem->setValue(desc);
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

std::string GuiMain::getText() {
    std::string itemText;
    switch (this->tinfoilReady) {
        case status::ready:
            itemText = "Tinfoil Ready";
            break;
        case status::notReady:
            itemText = "Not Tinfoil Ready";
            break;
        case status::error:
            itemText = "An error occurred";
            break;
        default:
            itemText = "Default case used. (This shouldn't happen)";
    }
    //return this->tinfoilReady == error ? "An Error Occurred" : this->tinfoilReady == ready ? "Tinfoil Ready" : "Not Tinfoil Ready";
    return itemText;
}

// Called once every frame to handle inputs not handled by other UI elements
/*virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
    return false;   // Return true here to singal the inputs have been consumed
}*/
#include "gui_main.hpp"
#include <tesla.hpp>
#include <sstream>
#include <fstream>

// Status Descriptions and unicode glyphs
constexpr const char *const descriptions[4] = {
    [0] = "Unwrapped | \uE14C",
    [1] = "Wrapped | \uE14B",
    [2] = "Both are Present | \uE150",
    [3] = "Neither are Present | \uE150",    
};

// Paths for files tinfoil looks for and their respective alternate paths for Cling Wrap
constexpr const char *const paths[2][2] = {
    [0] = {
        [0] = "/bootloader",
        [1] = "/_bootloade",
    },
    [1] = {
        [0] = "/atmosphere/kips",
        [1] = "/atmosphere/_kip"
    },
};

GuiMain::GuiMain() {
    Result rc = fsOpenSdCardFileSystem(&this->m_fs);
    if (R_FAILED(rc))
        return;

    // Loop through the list of possible paths
    for(int i = 0; i < (int) (sizeof(paths)/sizeof(paths[0])); i++) {
        // Declare the mainPath variable (the unaltered path)
        // Also declare the alt path variable
        const char *mainPath = paths[i][0];
        const char *alt = paths[i][1];
        // If either directory is present, continue
        if(this->FS_DirExists(&this->m_fs, mainPath) || this->FS_DirExists(&this->m_fs, alt)) {
            // Create a new directory for the current directory that is found
            directory currentDir = {
                .dirName = mainPath + 1,
                .listItem = new tsl::elm::ListItem(currentDir.dirName),
                .dirPath = mainPath,
                .altPath = alt
            };

            // Set a listener for each directory found that renames it when clicked.
            currentDir.listItem->setClickListener([this, currentDir](u64 keys) -> bool {
                if(keys & KEY_A) {
                    rename(currentDir);
                    return true;
                }
                return false;
            });

            // Add the current directory to the list of directory items
            this->directoryListItems.push_back(std::move(currentDir));
        }
    }
    
    // Once the loop has concluded, set the scanned variable to true
    this->scanned = true;
}

GuiMain::~GuiMain() { 
    fsFsClose(&this->m_fs);
}

// Called when this Gui gets loaded to create the UI
// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
tsl::elm::Element *GuiMain::createUI() {
    //writeToLog("Entered create UI function");

    std::stringstream subTitle;
    subTitle << VERSION << " by Acta";
    tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame("Cling Wrap", subTitle.str());

    // If no conflicting directories were found, display a warning to the user
    if(this->directoryListItems.size() == 0) {
        const char *description = this->scanned ? "No conflicting folders found!" : "Scan failed!";

        auto *warning = new tsl::elm::CustomDrawer([description](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("\uE150", false, 180, 250, 90, renderer->a(0xFFFF));
            renderer->drawString(description, false, 110, 340, 25, renderer->a(0xFFFF));
        });

        rootFrame->setContent(warning);
    } else {
        // If at least 1 directory was found

        // Create a new list element for the GUI
        tsl::elm::List *dirList = new tsl::elm::List();

        // Add a custom drawer item that displays definitions to the user
        dirList->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("\uE142  Wrapped = Ready for Tinfoil\n\uE142  Unwrapped = Ready for Boot", false, x + 5, y + 20, 15, renderer->a(tsl::style::color::ColorDescription));
        }), 50);

        // Create a wrap all button and apply a listener
        auto *wrapAllButton = new tsl::elm::ListItem("Wrap");
        wrapAllButton->setClickListener([this](u64 keys) -> bool { 
            if(keys & KEY_A) {
                renameAll(status::wrapped);
                return true;
            }
            return false;
        });
        // Add the new button to the list item on the GUI
        dirList->addItem(wrapAllButton);
        
        // Create an unwrap all button and apply a listener
        auto *unwrapAllButton = new tsl::elm::ListItem("Unwrap");
        unwrapAllButton->setClickListener([this](u64 keys) -> bool { 
            if(keys & KEY_A) {
                renameAll(status::unwrapped);
                return true;
            }
            return false;
        });
        // Add the new button to the list item on the GUI
        dirList->addItem(unwrapAllButton);

        // Add another custom drawer item that provides information to the user
        dirList->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("\uE142  These directories can be renamed individually.", false, x + 5, y + 20, 15, renderer->a(tsl::style::color::ColorDescription));
        }), 30);

        // Loop through the found directories and add their list items to the GUI list element
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

// Set the description based on the status of each directory
void GuiMain::updateStatus(const directory &dir) {

    const char *desc = descriptions[this->getStatus(dir)];
    dir.listItem->setValue(desc);
}

// Determine whether the directory is present
status GuiMain::getStatus(const directory &dir) {
    const char *path = dir.dirPath.c_str();
    const char *alt = dir.altPath.c_str();

    if(this->FS_DirExists(&this->m_fs, path) && this->FS_DirExists(&this->m_fs, alt)) {
        // Both directories are found
        return status::bothPresent;
    } else {
        if(this->FS_DirExists(&this->m_fs, path)) {
            return status::unwrapped;
        } else if(this->FS_DirExists(&this->m_fs, alt)) {
            return status::wrapped;
        } else {
            // No directories are found
            return status::neitherPresent;
        }
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

// Rename a directory
void GuiMain::rename(const directory &dir) {
    // Get the path variables
    const char *path = dir.dirPath.c_str();
    const char *alt = dir.altPath.c_str();
    
    if(this->FS_DirExists(&this->m_fs, path) && this->FS_DirExists(&this->m_fs, alt)) {
        // Both directories are found
    } else {
        if(this->FS_DirExists(&this->m_fs, path)) {
            if(FS_RenameDir(&this->m_fs, path, alt) == 0) {
                // Success
            }
        } else if(this->FS_DirExists(&this->m_fs, alt)) {
            if(FS_RenameDir(&this->m_fs, alt, path) == 0) {
                // Success
            }
        } else {
            // No directories are found
        }
    }    
}

// Rename all folders so that they are in the s status
void GuiMain::renameAll(status s) {
    // wrapped = alt path present
    // unwrapped = path present
    for (const auto &dir : this->directoryListItems) {
        const char *path = dir.dirPath.c_str();
        const char *alt = dir.altPath.c_str();
        if(s == status::wrapped) {
            if(this->FS_DirExists(&this->m_fs, path) && !this->FS_DirExists(&this->m_fs, alt)) {
                if(FS_RenameDir(&this->m_fs, path, alt) == 0) {
                    // Success
                }
            }
        } else if (s == status::unwrapped) {
            if(this->FS_DirExists(&this->m_fs, alt) && !this->FS_DirExists(&this->m_fs, path)) {
                if(FS_RenameDir(&this->m_fs, alt, path) == 0) {
                    // Success
                }
            }
        }
    }
}

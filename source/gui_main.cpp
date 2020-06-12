#include "gui_main.hpp"
#include <tesla.hpp>    // The Tesla Header
#include <sstream>
#include <fstream>
#include <filesystem>

namespace nxfs = std::filesystem;


GuiMain::GuiMain() {
    Result rc = fsOpenSdCardFileSystem(&this->m_fs);
    if (R_FAILED(rc))
        return;
        
    this->updateStatus();
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
    auto frame = new tsl::elm::OverlayFrame("Cling Wrap", VERSION);

    // A list that can contain sub elements and handles scrolling
    auto list = new tsl::elm::List();

    //writeToLog("Retreived tinfoil status");
    this->tinfoilReadyItem = new tsl::elm::ListItem(this->getText());
    /*tinfoilReadyItem->setClickListener([this](u64 keys) { 
        if (keys & KEY_A) {
            // Create char arrays for the old and new paths of files that need to be renamed
           // nxfs::path oldPath, newPath;

            //this->tinfoilReady = this->tinfoilReady == error ? error : this->tinfoilReady == ready ? notReady : ready;
            //this->tinfoilReadyItem->setText(this->getText());
            // Set the old and new path's according to the devices tinfoilReady status
            /*if(this->tinfoilReady) {
                //writeToLog("Determined that the folder is currently /_bootloader and therefore ready for tinfoil use.");
                oldPath = ALTBOOTLOADERPATH;
                newPath = BOOTLOADERPATH;
                //writeToLog("Prepared old and new path strings.");
                this->tinfoilReady = false;
            } else {
                //writeToLog("Determined that the folder is currently /bootloader and therefore not ready for tinfoil use.");
                oldPath = BOOTLOADERPATH;
                newPath = ALTBOOTLOADERPATH;
                //writeToLog("Prepared old and new path strings.");
                this->tinfoilReady = true;
            }
            
            nxfs::rename(oldPath, newPath);
            //writeToLog("Renaming successful.");
            
            return true;
        }

        return false;
    });*/

    list->addItem(tinfoilReadyItem);

    // Add the list to the frame for it to be drawn
    frame->setContent(list);
    
    // Return the frame to have it become the top level element of this Gui
    return frame;
}

// Called once every frame to update values
void GuiMain::update() {
    static u32 counter = 0;

    if (counter++ % 20 != 0) 
        return;
    
    //this->updateStatus();
}

void GuiMain::updateStatus() {
    //writeToLog("Checking if /bootloader or /_bootloader exists.");
    if(this->FS_DirExists(&this->m_fs, BOOTLOADERPATH)) {
        //writeToLog("/bootloader path exists");
        this->tinfoilReady = status::notReady;
    } else if (FS_DirExists(&this->m_fs, ALTBOOTLOADERPATH)) {
        //writeToLog("/_bootloader path exists");
        this->tinfoilReady = status::ready;
    } else {
        this->tinfoilReady = status::error;
    }
    //this->tinfoilReady = status::ready;
    //this->tinfoilReadyItem->setText(this->getText());
    //std::string text = this->getText();
    //this->tinfoilReadyItem->setText(text);
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
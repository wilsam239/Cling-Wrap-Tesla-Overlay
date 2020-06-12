#define TESLA_INIT_IMPL // If you have more than one file using the tesla header, only define this in the main one

#include <gui_main.hpp>

class ClingWrapOverlay : public tsl::Overlay {
  public:
    ClingWrapOverlay() {}
    ~ClingWrapOverlay() {}
                                             // libtesla already initialized fs, hid, pl, pmdmnt, hid:sys and set:sys
    virtual void initServices() override {}  // Called at the start to initialize all services necessary for this Overlay
    virtual void exitServices() override {}  // Callet at the end to clean up all services previously initialized

    virtual void onShow() override {}    // Called before overlay wants to change from invisible to visible state
    virtual void onHide() override {}    // Called before overlay wants to change from visible to invisible state

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<GuiMain>();  // Initial Gui to load. It's possible to pass arguments to it's constructor like this
    }
};

int main(int argc, char **argv) {
    return tsl::loop<ClingWrapOverlay>(argc, argv);
}
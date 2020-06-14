# Cling Wrap Tesla Overlay
<p align="center">
<img src = "https://user-images.githubusercontent.com/11286118/84357080-6e643c00-ac08-11ea-9435-19f116d0477e.png"\><br>
<img alt="GitHub All Releases" src="https://img.shields.io/github/downloads/wilsam239/Cling-Wrap-Tesla-Overlay/total">

A Nintendo Switch Tesla Overlay to enable Tinfoil to be run on devices with hekate (without the custom fork) by renaming folders that Tinfoil looks for on your SD card so that it can be run.

## How It Works
Tinfoil looks through your switch's files for specific folders that are used by hekate. This overlay renames them temporarily so that you can start tinfoil without uninstalling Hekate.

After renaming, it is important to use the overlay again to rename the files so that hekate can boot.

Specifically, it renames the `bootloader` folder.

In the event that the app crashes during use, be sure to rename the files manually by inserting your SD card into a computer. Also be sure to upload your log.txt file (found in the Cling Wrap nro folder) as an issue on GitHub.

## How to install
Copy the `ovlClingWrap.ovl` file to `/switch/.overlays/` in your sdcard

## Screenshots


## Thanks
- KiteAffair for the name suggestion
- NX-Shell for their filesystem code
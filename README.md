# 🐾 Tolitica

<img src="https://images2.imgbox.com/12/9a/8fx4ttgZ_o.png" alt="Tolitica Logo" width="300"/>

## 🚀 Overview
Tolitica is the Xray_OS Welcome Application and assistant, designed to facilitate interactions and tweaking the system. It serves as the Official Xray_OS main helper, allowing users to perform complicated system operations with just a click of a button. Tolitica can handle tasks like changing the User's Shell, enabling/disabling Flatpak/Snapd, disabling Terminal Theming, and enabling/disabling AppArmor. It provides peace of mind when it comes to system configuration.

## 🛠️ Framework and Language
Tolitica is built using C++ and the QT Framework.

## 🔧 How to Contribute
Contributing to Tolitica is straightforward! Here’s how you can get started:

1. **Install a Text Editor**: Use a text editor like VS Code or Zed to open the project.
2. **Open the Project**: Clone the repository and open it in your preferred text editor.
3. **Ensure Dependencies**: Make sure you have the proper QT libraries and dependencies installed. For Arch Linux, you can install them with:
   ```bash
   sudo pacman -S qt5-base qt5-tools qt5-declarative qt5-creator qt6-wayland

## 📥 Build and Run

## Clone the repository
git clone https://github.com/Xray-OS/Tolitica.git
cd Tolitica

## Create a build directory
mkdir build
cd build

## Set CMAKE properly
cmake .. -DCMAKE_PREFIX_PATH=$(qmake -query QT_INSTALL_PREFIX)/lib/
cmake

## If you’re using Qt6, you might need:
cmake .. -DCMAKE_PREFIX_PATH=$(qtpaths --qt-version 6 --install-prefix)/lib/cmake

## Build the project
cmake --build .

## Run Tolitica
./Tolitica
## or with sudo for elevated permissions
sudo ./Tolitica

## 📋 License
Tolitica is open-source and available under the Xray License.

🌐 Contact
For any questions or contributions, feel free to open an issue or pull request on the GitHub repository.

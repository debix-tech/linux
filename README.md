### System SDK Download
- Ubuntu 22.04 :    
     https://github.com/nxp-imx/meta-nxp-desktop
- Yocto-Linux 6.1.22_2.0.0    
     https://www.nxp.com/design/software/embedded-software/i-mx-software/embedded-linux-for-i-mx-applications-processors:IMXLINUX?   
     
### Building the Kernel
The default compilers and linkers that come with an OS are configured to build executables to run on that OS - they are native tools - but that doesn’t have to be the case. A cross-compiler is configured to build code for a target other than the one running the build process, and using it is called cross-compilation.

Cross-compilation of Debix kernel is useful for two reasons:

- it allows a 64-bit kernel to be built using a 32-bit OS, and vice versa, and
- even a modest laptop can cross-compile a Debix kernel significantly faster than the Debix itself.

The instructions below are divided into native builds and cross-compilation; choose the section appropriate for your situation - although there are many common steps between the two, there are also some important differences.
#### 1.0 Building the Kernel Locally
On a Debix, first install the latest version of Debix OS. Then boot your Debix, plug in Ethernet to give you access to the sources, and log in.

==Note==   
        
    kernel compilation occupies much space, thus,we recommend using TF card larger than 32G.   
 

###### First install git and the build dependencies:
     sudo apt install git bc bison flex libssl-dev make  
     
###### Next get the sources, which will take some time:
    git clone --depth=1 https://github.com/debix-tech/linux -b Debix-L6.1.22
    
###### Choosing Sources
The git clone command above will download the current active branch (the one we are building Debix OS images from) without any history. Omitting the --depth=1 will download the entire repository, including the full history of all branches, but this takes much longer and occupies much more storage.   
To download a different branch (again with no history), use the --branch option： 
    
    git clone --depth=1 --branch <branch> https://github.com/debix-tech/linux
where <branch> is the name of the branch that you wish to download.

Refer to the original GitHub repository for information about the available branches.

###### Kernel Configuration
Configure the kernel; as well as the default configuration, you may wish to configure your kernel in more detail or apply patches from another source, to add or remove required functionality.

###### Apply the Default Configuration
First, prepare the default configuration by running the following commands：

The default configuration of Debix is imx_v8_defconfig  

    cd debix-kernel
    make imx_v8_defconfig
    
###### Building the Kernel
Build and install the kernel, modules, and Device Tree blobs; this step takes a relatively long time ;    
The default device tree is imx8mp-evk.dts, according to the add-on boards used and the show requirement, different device tree can be chosen through Add-on Board tool.

```
make -j4 
sudo make INSTALL_MOD_STRIP=1 modules_install
sudo cp arch/arm64/boot/dts/freescale/*.dtb /boot/. 
sudo cp arch/arm64/boot/Image /boot/.
```

#### 1.1 Cross-Compiling the Kernel
First, you will need a suitable Linux cross-compilation host. We tend to use Ubuntu; since Debix OS is also a Debian distribution, it means many aspects are similar, such as the command lines.   

You can either do this using VirtualBox (or VMWare) on Windows, or install it directly onto your computer. For reference, you can follow instructions online at Wikihow.  

###### Install Required Dependencies and Toolchain
To build the sources for cross-compilation, make sure you have the dependencies needed on your machine by executing：

    sudo apt install git bc bison flex libssl-dev make libc6-dev libncurses5-dev
    
If you find you need other things, please submit a pull request to change the documentation.

Get cross compilation tool

```
sudo mkdir /opt/toolchain
cd /opt/toolchain
sudo wget https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-a/9.2-2019.12/binrel/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
tar xpf gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
export PATH=$PATH:/opt/toolchain/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin
```
==Note==

```
You need to execute the following command before do compilation.
export PATH=$PATH:/opt/toolchain/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin
```
###### Get the Kernel Sources
To download the minimal source tree for the current branch, run:  

     git clone --depth=1 https://github.com/debix-tech/linux
     
See Choosing sources above for instructions on how to choose a different branch
###### Build sources
Enter the following commands to build the sources and Device Tree files


```
export PATH=$PATH:/opt/toolchain/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin
cd debix-kernel 
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-  imx_v8_defconfig
```
Build with Configs   
==Note==

    To speed up compilation on multiprocessor systems, and get some improvement on single processor ones, use -j n, where n is the number of processors * 1.5. You can use the nproc command to see how many processors you have. Alternatively, feel free to experiment and see what works!


Compile kernel dts modules

==Note==

    You should be in the debix-kernel repository directory before executing the following commands


```
export PATH=$PATH:/opt/toolchain/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-  
```

###### Install Directly onto the SD Card
Having built the kernel, you need to copy it onto your Debix and install the modules; this is best done directly using an SD card reader. Prepare an SD card with Debix OS installed beforehand.

First, use lsblk before and after plugging in your SD card to identify it. You should end up with something a lot like this：
    
    sdb
      sdb1
      sdb2
     
with sdb1 being the FAT (boot) partition, and sdb2 being the ext4 filesystem (root) partition.  

==Note==

```
You should be in the first level of debix-kernel repository directory before executing the following commands,i.e., After you run pwd|grep .*debix-kernel$,you could get output like this: 
/home/debix/debix-kernel
```

Mount these first, adjusting the partition number as necessary:

```
mkdir mnt
mkdir mnt/fat32
mkdir mnt/ext4
sudo mount /dev/sdb1 mnt/fat32
sudo mount /dev/sdb2 mnt/ext4
```
==Note==

```
You should adjust the drive letter appropriately for your setup, e.g. if your SD card appears as /dev/sdc instead of /dev/sdb.
You should be in the first level of debix-kernel repository directory before executing the following commands,i.e., After you run pwd|grep .*debix-kernel$,you could get output like this: 
/home/debix/debix-kernel
```
Next, install the kernel modules onto the SD card:

```
export PATH=$PATH:/opt/toolchain/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin 
sudo env PATH=$PATH make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-  INSTALL_MOD_PATH=mnt/ext4 INSTALL_MOD_STRIP=1 modules_install
sudo umount mnt/ext4
```

Finally, copy the kernel and Device Tree blobs onto the SD card, making sure to back up your old kernel:


```
sudo cp mnt/fat32/Image mnt/fat32/Image-backup.img
sudo cp arch/arm64/boot/Image mnt/fat32/Image
sudo cp arch/arm64/boot/dts/freescale/*.dtb mnt/fat32/. 
sudo umount mnt/fat32
```

Finally, plug the card into Debix and boot it!

#### 2.0 Configuring the Kernel
The Linux kernel is highly configurable; advanced users may wish to modify the default configuration to customize it to their needs, such as enabling a new or experimental network protocol, or enabling support for new hardware.

Configuration is most commonly done through the make menuconfig interface. Alternatively, you can modify your .config file manually, but this can be more difficult for new users.
#### 2.1 Preparing to Configure
The menuconfig tool requires the ncurses development headers to compile properly. These can be installed with the following command:
 
    sudo apt install libncurses5-dev
    
You’ll also need to download and prepare your kernel sources, as described in the build guide. In particular, ensure you have installed the default configuration.
#### 2.2 Using menuconfig

Once you’ve got everything set up and ready to go, you can compile and run the menuconfig utility as follows:  
  
     make menuconfig

Or, if you are cross-compiling a 64-bit kernel:

```
export PATH=$PATH:/opt/toolchain/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- menuconfig
```

The menuconfig utility has simple keyboard navigation. After a brief compilation, you’ll be presented with a list of submenus containing all the options you can configure; there’s a lot, so take your time to read through them and get acquainted.

Use the arrow keys to navigate, the Enter key to enter a submenu (indicated by --->), Escape twice to go up a level or exit, and the space bar to cycle the state of an option. Some options have multiple choices, in which case they’ll appear as a submenu and the Enter key will select an option. You can press h on most entries to get help about that specific option or menu.

Resist the temptation to enable or disable a lot of things on your first attempt; it’s relatively easy to break your configuration, so start small and get comfortable with the configuration and build process.

#### 3.0 Saving your Changes
Once you’ve done making the changes you want, press Escape until you’re prompted to save your new configuration. By default, this will save to the .config file. You can save and load configurations by copying this file around.
 


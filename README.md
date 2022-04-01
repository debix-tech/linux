### debix kernel build
#### 1. 下载内核   

```
git clone https://github.com/9278978/polyhex-kernel.git
```
#### 2. 下载编译器 
- https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-a/9.2-2019.12/binrel/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
- linux 下载命令 
```
...$ wget https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-a/9.2-2019.12/binrel/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
...$ cp gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz  /opt/toolchain
...$ cd /opt/toolchain
...$ tar xpf gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
```

#### 3. 配置环境 

```
...$ export PATH=$PATH:/opt/toolchain/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin
...$ export ARCH=arm64
...$ export CROSS_COMPILE=aarch64-none-linux-gnu-
```

#### 4. 开始编译Image和Dts

```
...$ make imx_v8_defconfig
...$ make -j32 Image
...$ make -j32 freescale/imx8mp-evk.dtb
...$ make -j32 freescale/imx8mp-evk-raspberrypi-touchscreen.dtb
```

#### 5. 创建out目录和拷贝Dtb和Image

```
...$ mkdir out
...$ cp arch/arm64/boot/Image out
...$ cp arch/arm64/boot/dts/freescale/imx8mp-evk.dtb out
```

#### 6. 编译modules

```
...$ make INSTALL_MOD_PATH=out INSTALL_MOD_STRIP=1 modules_install
```

#### 7. 拷贝新编译好的 Image 、 dtb 和modules 到 debix 设备
- 在 第5、6步已经把文件拷贝到out目录，只要把out 目录拷贝只debix设备

```
...$ sudo mount /dev/mmcblk1p1 /boot
...$ sudo cp image-out/Image /boot/. 
...$ sudo cp image-out/imx8mp-evk.dtb  /boot/.
...$ sudo cp -ar image-out/lib/modules/* /lib/modules/.
```

#### 8. 重启完成kernel更新


source /opt/fsl-imx-xwayland/6.1-mickledore/environment-setup-armv8a-poky-linux
export ARCH=arm64
#make distclean
make imx_v8_defconfig
make -j32

if [ $? != 0 ] ; then
	echo "build kernel err !!!"
	return
fi
rm -rf image_out
mkdir -p image_out/boot
#cp arch/arm64/boot/dts/freescale/imx93-11x11-evk.dtb image_out/boot/.
#cp arch/arm64/boot/dts/freescale/imx93-debix-base.dtb image_out/boot/.
#cp arch/arm64/boot/dts/freescale/imx93-debix-HC101IK25050-D59V.C.dtb image_out/boot/imx93-11x11-evk.dtb
#cp arch/arm64/boot/dts/freescale/imx93-debix-HC101IK25050-D59V.C.dtb image_out/boot/.
#cp arch/arm64/boot/dts/freescale/imx93-debix-HC080IY28026-D60.C.dtb image_out/boot/.
cp arch/arm64/boot/dts/freescale/*.dtb image_out/boot/.

cp arch/arm64/boot/Image image_out/boot/. 

#return

make -j32 modules

if [ $? != 0 ] ; then
	echo "build modules err !!!"
	return
fi

make INSTALL_MOD_PATH=image_out INSTALL_MOD_STRIP=1 modules_install

tar cpf image_out.tar image_out

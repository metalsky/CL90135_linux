#imposta path
export PATH="/opt/mv_pro_5.0/montavista/pro/devkit/arm/v5t_le/bin:/opt/mv_pro_5.0/montavista/pro/bin:/opt/mv_pro_5.0/montavista/common/bin:$PATH" 

#cambia nella directory del progetto
cd /home/bagside/workdir/lsp/ti-davinci/linux-2.6.18_pro500 

#per pulire le options del kernel
make ARCH=arm CROSS_COMPILE=arm_v5t_le- distclean
make ARCH=arm CROSS_COMPILE=arm_v5t_le- da830_omapl137_defconfig

#per editare le options del kernel
make ARCH=arm CROSS_COMPILE=arm_v5t_le- menuconfig

#build uImage
make -j4 ARCH=arm CROSS_COMPILE=arm_v5t_le- uImage 

#build Modules
make -j4 ARCH=arm CROSS_COMPILE=arm_v5t_le- modules 


#Installing Modules to Target File System 
make ARCH=arm CROSS_COMPILE=arm_v5t_le- INSTALL_MOD_PATH=/home/bagside/workdir/filesys modules_install

# Linux Kernel Image to TFTP directory
# ATTENZIONE! IN REALTA' TFTPBOOT USA LA DRECTORY /var/lib/tftpboot PER DEFAULT
# VERIFICARE NEL FILE /etc/default/atftpd QUAL E' LA DIRECTORY DI BOOT TFTP
cp /home/bagside/workdir/lsp/ti-davinci/linux-2.6.18_pro500/arch/arm/boot/uImage /var/lib/tftpboot
chmod a+r /var/lib/tftpboot/uImage 


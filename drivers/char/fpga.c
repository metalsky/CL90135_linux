/*
 *  linux/drivers/char/fpga.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  Added devfs support. 
 *    Jan-11-1998, C. Scott Ananian <cananian@alumni.princeton.edu>
 *  Shared /dev/zero mmaping support, Feb 2000, Kanoj Sarcar <kanoj@sgi.com>
 */

#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/smp_lock.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/crash_dump.h>
#include <linux/backing-dev.h>
#include <linux/bootmem.h>
#include <linux/pipe_fs_i.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#ifdef CONFIG_IA64
# include <linux/efi.h>
#endif

//#define defDebugPrints_FpgaDriver

// Minor version fpga...
#define FPGA_MINOR 1
struct cdev *dev_fpga;

static int FPGA_mmap(struct file * filp, struct vm_area_struct * vma){
    int retval;

    unsigned long physical  = 0;
    unsigned long psize     = 0;
    unsigned int  mode_io   = 0;
    size_t        vsize     = vma->vm_end - vma->vm_start;

#ifdef defDebugPrints_FpgaDriver
    printk( "+ Fpga mmap\n" );
#endif      
    
      
    physical  = 0x60000000;
    psize     = 4096;
    mode_io   = 1;

    if( vsize > psize )
    {
        printk( "Fpga: vsize[%d] > psize[%ld] mapping to pinmux area\n", vsize, psize );
        return -ENOMEM;
    }

    // Make sure the memory is not cached
    vma->vm_page_prot = pgprot_noncached( vma->vm_page_prot );
    vma->vm_page_prot = pgprot_writecombine( vma->vm_page_prot );
    vma->vm_flags |= VM_IO;
#ifdef defDebugPrints_FpgaDriver
    printk( " Fpga: mapping io\n" );
#endif     
    retval = remap_pfn_range( vma, vma->vm_start,physical >> PAGE_SHIFT, vsize, vma->vm_page_prot );
    if( 0 != retval )
    {
        printk( "Fpga: unable to remap_pfn_range: %d\n", retval );
    }

#ifdef defDebugPrints_FpgaDriver
    printk( "- Fpga mmap\n" );
#endif      
      return 0;
}
  
static int FPGA_open(struct inode * inode, struct file * filp)
{
    return 0; // all ok
}
  
static struct file_operations fpga_fops =
{
  .owner = THIS_MODULE,
  .mmap  = FPGA_mmap,
  .open  = FPGA_open,
};
    
static int fpga_class_open(struct inode * inode, struct file * filp)
{
	switch (iminor(inode)) {
		case 1:
		default:
			filp->f_op = &fpga_fops;
			break;
	}
	if (filp->f_op && filp->f_op->open)
		return filp->f_op->open(inode,filp);
	return 0;
}
    
    
static const struct file_operations fpga_class_fops = {
	.open		= fpga_class_open,	/* just a selector for the real open */
};

static const struct {
	unsigned int	minor;
	char			*name;
	umode_t			mode;
	const struct file_operations	*fops;
} devlist[] = { /* list of minor devices */
	{1, "fpga",     S_IRUSR | S_IWUSR | S_IRGRP, &fpga_fops},
};   

static struct class *fpga_class;

static int __init fpga_init_module ( void )
{
    int i;
    if (register_chrdev(FPGA_MAJOR,"fpga",&fpga_class_fops))
	    printk("unable to get major %d for fpga devs\n", FPGA_MAJOR);
    fpga_class = class_create(THIS_MODULE, "fpga");
    for (i = 0; i < ARRAY_SIZE(devlist); i++)
	    class_device_create(fpga_class, NULL,
				    MKDEV(FPGA_MAJOR, devlist[i].minor),
				    NULL, devlist[i].name);

	return (0);
}

static void __exit fpga_cleanup_module ( void )
{
  cdev_del( dev_fpga );
}



fs_initcall(fpga_init_module);

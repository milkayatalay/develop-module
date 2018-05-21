#include <linux/module.h>  // Needed by all modules
#include <linux/kernel.h>  // Needed for KERN_INFO
#include <linux/fs.h>      // Needed by filp
#include <asm/uaccess.h>   // Needed by segment descriptors
#include <linux/uaccess.h> // Needed by segment descriptors
#include <linux/slab.h>    // Needed for kmalloc
#include <linux/device.h>  // Needed for device driver 
#include <linux/cdev.h>	   // Needed for character device


static char msg[2];
static char bosluk[2] = {'\0','\0'};

struct tty_driver *tty;

static dev_t first;
static struct class *cl;
static struct cdev c_dev;

char buf[256];


////////////////////////////////////////////////////////////////////
/*--------------------- Main Functions ---------------------------*/
////////////////////////////////////////////////////////////////////


void yaz(char* destination){

  struct file *f;
  mm_segment_t fs;
  char *pet;

  pet = (char*)kmalloc(sizeof(char)*20,GFP_KERNEL); 
  sprintf(pet,"/dev/ttyUSB%c",msg[0]);
  

  f = filp_open(pet, O_RDONLY, 0);
  if(f == NULL)
      printk(KERN_ALERT "filp_open error!!.\n");
  else{

      fs = get_fs();
      set_fs(get_ds());
      f->f_op->read(f, buf, 256, &f->f_pos);
      set_fs(fs);

      sprintf(destination,buf);
     
  }

  filp_close(f,NULL);
  kfree(pet);
}


////////////////////////////////////////////////////////////////////
/*------------------- Device Functions ---------------------------*/
////////////////////////////////////////////////////////////////////


/* Device açılınca tetiklenen fonksiyon*/
static int c_open(struct inode *i,struct file *f){
  return 0;
}

/*Device kapanınca tetiklenen fonksiyon*/
static int c_release(struct inode *i,struct file *f){
  return 0;
}

/*Devicedan okuma yapılacagı zaman tetiklenen fonksiyon*/
static ssize_t c_read(struct file *f, char __user *buf, size_t
len, loff_t *off){
  ssize_t ret;
  size_t str_len = 10;
  char *temp;

  temp = (char*)kmalloc(sizeof(char)*20,GFP_KERNEL); 

  yaz(buf);

  sprintf(temp,"nabiz: %s/n",buf);
  printk(KERN_INFO "nabiz: %s",buf);

  ret = simple_read_from_buffer(buf,len,off,temp,str_len);
  //ret = 256;
  kfree(temp);
  return ret;
}

/*Device a parametre gönderileceği zaman tetiklenen fonksiyon*/
static ssize_t c_write(struct file *f,const char __user *buf, size_t
len, loff_t *off){
  ssize_t ret ;

  if(len > 3)
    len = 3;

  strcpy(msg,bosluk);
  memcpy(msg,buf,len); // aldığımız inputu msg değişkenine koy
  //isle(msg); //bu değişkeni parse edip gerekli yerlere yönlendir
  ret = strlen(msg);

  return len;
}

//Deviceın kontrol fonksiyonlarını tanıt
static struct file_operations fops ={
  .owner = THIS_MODULE,
  .open = c_open,
  .read = c_read,
  .release = c_release,
  .write = c_write
};


////////////////////////////////////////////////////////////////////
/*------------------- Module Functions ---------------------------*/
////////////////////////////////////////////////////////////////////


/*Modülün init fonksiyonu*/
static int __init firs_init(void) {
  int i;

  //character device a yer ayır
  if(alloc_chrdev_region(&first,0,1,"DevelOp")<0){
    return -1;
  }

  if((cl = class_create(THIS_MODULE,"dvlOp"))== NULL){
    unregister_chrdev_region(first,1);
    return -1;
  }

 // device ı oluştur
  if(device_create(cl,NULL,first,NULL,"DevelOpNull") == NULL){
    class_destroy(cl);
    unregister_chrdev_region(first,1);
    return -1;
  }

  for(i=0;i<128;i++)
      buf[i] = 0;

  //Device ı başlat
  cdev_init(&c_dev,&fops);

  //Device ı ekle
  if(cdev_add(&c_dev,first,1) ==  -1){
    device_destroy(cl,first);
    class_destroy(cl);
    unregister_chrdev_region(first,1);
    return -1;
  }


  printk(KERN_INFO "\nDevelOp registered\n");
  return 0;
}

/*Modülün cleanup fonksiyonu*/
static void __exit first_exit(void) {

  //Device ı kaldır
  cdev_del(&c_dev);
  device_destroy(cl,first);
  class_destroy(cl);
  unregister_chrdev_region(first,1);

  printk(KERN_INFO "\nDevelOp unregistered\n");
}

MODULE_LICENSE("GPL");
module_init(firs_init);
module_exit(first_exit);

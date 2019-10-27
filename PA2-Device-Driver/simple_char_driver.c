

#include<linux/init.h>
#include<linux/module.h>

#include<linux/fs.h>
#include<linux/slab.h>
//#include<asm/uaccess.h>
//#include<linux/sched.h>
//#include<linux/cdev.h>
#include<linux/uaccess.h>

#define BUFFER_SIZE 1024

/* Define device_buffer and other global data structures you will need here */
#define DEVICE_NAME "TangChar"
#define MAJOR_NUMBER 266

static int OpenNumber=0;
static int CloseNumber=0;
static char *device_buffer;



ssize_t simple_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t simple_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);
int simple_char_driver_open (struct inode *pinode, struct file *pfile);
int simple_char_driver_close (struct inode *pinode, struct file *pfile);
loff_t simple_char_driver_seek (struct file *pfile, loff_t offset, int whence);
static int simple_char_driver_init(void);
static void simple_char_driver_exit(void);

struct file_operations simple_char_driver_file_operations;
//struct task_struct;

struct file_operations simple_char_driver_file_operations = {

  .owner   = THIS_MODULE,
  .read=simple_char_driver_read,
  .write=simple_char_driver_write,
  .open=simple_char_driver_open,
  .release=simple_char_driver_close,
  .llseek=simple_char_driver_seek,
  /* add the function pointers to point to the corresponding file operations. look at the file fs.h in the linux souce code*/
};


ssize_t simple_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer to where you are writing the data you want to be read from the device file*/
	/* length is the length of the userspace buffer*/
	/* offset will be set to current position of the opened file after read*/
	/* copy_to_user function: source is device_buffer and destination is the userspace buffer *buffer */
  int error_count=0;
  if(BUFFER_SIZE<=*offset)
  {
    printk(KERN_ALERT "TangChar: Reading Error! The offset is no less than the total size \n");
    return 0;
  }
  size_t copy_len;
  if(BUFFER_SIZE-*offset<length)
  	copy_len=BUFFER_SIZE-*offset;
  else
  	copy_len=length;
  error_count=copy_to_user(buffer,device_buffer+*offset,copy_len);
  int real_number=copy_len - error_count;
  if(error_count)
  {
    printk(KERN_ALERT "TangChar: Reading Error! Only %d characters are sent \n",real_number);
    //return -EFAULT;
  }
  else{
    printk(KERN_ALERT "TangChar: Sent %d characters to the user. \n", copy_len);
  }
  *offset+=real_number;
	return real_number;
}



ssize_t simple_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer where you are writing the data you want to be written in the device file*/
	/* length is the length of the userspace buffer*/
	/* current position of the opened file*/
	/* copy_from_user function: destination is device_buffer and source is the userspace buffer *buffer */
  int error_count=0;
  if(BUFFER_SIZE<=*offset)
  {
    printk(KERN_ALERT "TangChar: Reading Error! The offset is no less than the total size \n");
    return 0;
  }
  //size_t copy_len=min(BUFFER_SIZE-*offset,length);
  size_t copy_len;
  if(BUFFER_SIZE-*offset<length)
  	copy_len=BUFFER_SIZE-*offset;
  else
  	copy_len=length;
  error_count=copy_from_user(device_buffer+*offset,buffer,copy_len);
  int real_number=copy_len - error_count;
  if(error_count)
  {
    printk(KERN_ALERT "TangChar: Writing Error! Only %d characters are sent \n",real_number);
    //return -EFAULT;
  }
  else{
    printk(KERN_ALERT "TangChar: Sent %d characters to the kernel. \n", copy_len);
  }
  *offset+=real_number;
  return real_number;


	//return 0;
}


int simple_char_driver_open (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is opened and also print the number of times this device has been opened until now*/
        OpenNumber++;
        printk(KERN_ALERT "TangChar: Tangchar is opened, the opened time is %d \n",OpenNumber);
	return 0;
}

int simple_char_driver_close (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is closed and also print the number of times this device has been closed until now*/
        CloseNumber++;
        printk(KERN_ALERT "TangChar: Tangchar is closed, the closed time is %d \n",CloseNumber);
	return 0;
}

loff_t simple_char_driver_seek (struct file *pfile, loff_t offset, int whence)
{
	/* Update open file position according to the values of offset and whence */
        loff_t new_position=0;
        if(whence==0){
           new_position=offset;
        }
       else if(whence==1){
           new_position=pfile->f_pos+offset;
       }
       else if(whence==2){
           new_position=BUFFER_SIZE-offset;
       }
       else{
           //new_position=pfile->f_pos;
           //printk(KERN_ALERT "TangChar: ERROR! \n");
           return -EINVAL;
       }
       if(new_position<0 || new_position>BUFFER_SIZE)
           return -EINVAL;
       pfile->f_pos=new_position;
       return new_position;
}


static int simple_char_driver_init(void)
{
	/* print to the log file that the init function is called.*/
         
	/* register the device */
        register_chrdev(266,DEVICE_NAME,&simple_char_driver_file_operations);
        device_buffer=kmalloc(BUFFER_SIZE,GFP_KERNEL);
        printk(KERN_ALERT "TangChar: Initializing the TangChar LKM \n");
	return 0;
}

static void simple_char_driver_exit(void)
{
	/* print to the log file that the exit function is called.*/
        printk(KERN_ALERT "TangChar: Exiting the TangChar LKM \n");
        kfree(device_buffer);
	/* unregister  the device using the register_chrdev() function. */
        unregister_chrdev(DEVICE_NAME,MAJOR_NUMBER);
}
module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);

/* add module_init and module_exit to point to the corresponding init and exit function*/

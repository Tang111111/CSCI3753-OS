#include<linux/uaccess.h>
#include<linux/kernel.h>
#include<linux/linkage.h>
asmlinkage long sys_cs3753_add(int a,int b,int *c)
{
          printk(KERN_ALERT "ADD FUNCTION\n");
          printk(KERN_ALERT "The two numbers to be added is %d, %d\n",a,b);
          int d=a+b;
         // printk(KERN_INFO "The add result is %d\n",*c);
          copy_to_user(c,&d,4);
          printk(KERN_ALERT "The add result in syscall is %d\n",d);
          return 0;
}

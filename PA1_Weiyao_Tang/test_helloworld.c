
#include<unistd.h>
#include<sys/syscall.h>
#include<stdio.h>

int main(){
printf("Test Helloworld! \n");
syscall(333);
return 0;
}

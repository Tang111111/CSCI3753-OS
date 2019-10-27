#include<unistd.h>
#include<sys/syscall.h>
#include<stdio.h>

int main(){
int a=100;
int b=20;
int *number;
printf("The result in test function should be %d",a+b);
syscall(334,a,b,number);
return 0;
}


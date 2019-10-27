#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdbool.h>

static char *device_buffer;
#define BUFFER_SIZE 1024

int main(){
	int fd=open("/dev/simple_character_device",O_RDWR);
	char command;
	int ret=0;
	printf("Starting device test code example\n");
	//printf("%d\n",&fd);
	if(fd<0){
		perror("Failed to open the device ...\n");
		return errno;
	}

    
    bool flag=true;
    while(flag){
    	printf("Enter command\n");
    	scanf("%c",&command);
    	//printf("%c\n",command );
    	//fflush(stdin);
    	switch(command){
    		case 'r':
    		printf("Enter the number of bytes you want to read:\n");
            int number=0;
            scanf("%d",&number);
            device_buffer=(char*)malloc(number);
            ret=read(fd,device_buffer,number);
            if(ret<0){
              perror("Failed to read the message from the device.\n");
              //return errno;
            }
            printf("Date read from the device: \n");
            printf("%s\n",device_buffer);
            //fflush(stdin);
            break;
            
            case 'w':
    	    printf("Enter the data you want to write to the device\n");
    	    char stringToSend[BUFFER_SIZE];
    	    getchar();
    	    scanf("%[^\n]%*c",stringToSend);
    	    //printf("%s\n",stringToSend );
            ret=write(fd,stringToSend,strlen(stringToSend));
            if(ret<0){
              perror("Failed to write the message to the device.\n");
              //return errno;
            }
            //fflush(stdin);
            break;

            case 's':
            printf("Enter an offset value:\n");
            int offset_value=0;
    	    scanf("%d",&offset_value);
    	    printf("Enter a value for whence(third parameter):\n");
    	    int whence=0;
    	    //fflush(stdin);
    	    scanf("%d",&whence);
    	    ret=llseek(fd,offset_value,whence);
            if(ret<0 || ret >1024){
              printf("Failed to seek into the device.\n");
              break;
            }
            //fflush(stdin);
            printf("New position is %d\n", ret);
            break;

            case 'e':
            printf("Exit the Test App\n");
    	    flag=false;
    	    break;

    	    default:
    	    printf("You could only input r/w/s/e \n");
    	    break;
    	}
    	if(command!='w')
    		getchar();
    }
    /*while(flag){
    	printf("Enter command\n");
    	scanf("%c",&command);
    	//command=getchar();
    	//printf("The char is %s \n",&command);
        if(command =='r'){
        	getchar();
       int number=0;
       printf("Enter the number of bytes you want to read:\n");
       scanf("%d",&number);
       device_buffer=(char*)malloc(number);
       ret=read(fd,device_buffer,number);
       if(ret<0){
         perror("Failed to read the message from the device.\n");
         return errno;
       }
       printf("Date read from the device: \n");
       printf("%s\n",device_buffer);

    }else if(command == 'w'){
    	getchar();
    	printf("Enter the data you want to write to the device\n");
    	char stringToSend[BUFFER_SIZE];
    	scanf("%[^\n]%*c",stringToSend);
        ret=write(fd,stringToSend,strlen(stringToSend));
        if(ret<0){
         perror("Failed to write the message to the device.\n");
         return errno;
       }
       //while(getchar()!=='\n')
       //{

       //}

    }else if(command == 's'){
    	getchar();
    	int offset_value=0;
    	int whence=0;
    	printf("Enter an offset value:\n");
    	scanf("%d",&offset_value);
    	printf("Enter a value for whence(third parameter):\n");
    	scanf("%d",&whence);
    	ret=llseek(fd,offset_value,whence);
        if(ret){
         perror("Failed to seek into the device.\n");
         return errno;
       }

    }
    else if(command == 'e'){
    	getchar();
    	flag=false;
    	printf("Exit the Test App\n");
    }
    else{
    	//getchar();
       printf("You could only input r/w/s/e \n");
    }

    }*/
    
    return 0;
}

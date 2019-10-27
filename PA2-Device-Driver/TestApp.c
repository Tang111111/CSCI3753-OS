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
	if(fd<0){
		perror("Failed to open the device ...\n");
		return errno;
	}

    
    bool flag=true;
    while(flag){
    	printf("Enter command\n");
    	scanf("%c",&command);

    	switch(command){
    		case 'r':
    		printf("Enter the number of bytes you want to read:\n");
        int number=0;
        scanf("%d",&number);

        if(number>0){
            device_buffer=(char*)malloc(number);
            ret=read(fd,device_buffer,number);
            if(ret<0){
              perror("Failed to read the message from the device.\n");
            }
             printf("Date read from the device: \n");
             printf("%s\n",device_buffer);
             free(device_buffer);
          }
          else{
            printf("Number should be greater than 0\n");
          }    
            break;
            
            case 'w':
            printf("Enter the data you want to write to the device\n");
    	      char stringToSend[BUFFER_SIZE];
    	      getchar();
    	      scanf("%[^\n]%*c",stringToSend); //avoid whtiespace
            ret=write(fd,stringToSend,strlen(stringToSend));
            if(ret<0){
              perror("Failed to write the message to the device.\n");
            }
            break;

            case 's':
            printf("Enter an offset value:\n");
            int offset_value=0;
    	      scanf("%d",&offset_value);
    	      printf("Enter a value for whence(third parameter):\n");
    	      int whence=0;
    	      scanf("%d",&whence);
    	      ret=llseek(fd,offset_value,whence);
            if(ret<0 || ret >1024){
              printf("Failed to seek into the device.\n");
              break;
            }
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

    return 0;
}

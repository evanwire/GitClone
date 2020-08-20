#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(){

    system("mkdir clientside");
    system("mkdir serverside");
    system("make");
    system("mv WTFserver serverside");
    system("mv WTF clientside");
    int forkid = fork();
    
    //fork failed
    if(forkid<0){
        printf("Error occured while trying to test");
    }

    //child process
    if(forkid==0){
        system("./clientside/WTF configure 127.0.0.1 8888");
        system("./clientside/WTF create projectA");
        system("./clientside/WTF create projectA");
        system("./clientside/WTF destroy projectA");
        system("./clientside/WTF destroy projectA");
    }

    //parent process
    if(forkid>0){
        system("./serverside/WTFserver 8888");
    }

      
    return 0;
}

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <dirent.h>
#include <openssl/md5.h>

//Helper Methods
void checkArgs(int, char**);
void configure(char*, char*);
void create(char*);
char* connect_to_WTFserver(char*);
void destroy(char*);
void add(char*, char*);
char* hashIt(char*);
char* readFile(char*);
void copyFile(char*, int);
char* read_line(int);
void remove_file(char*, char*);
void currentVersion(char* projectName);
void checkout(char*);
void commit(char*);
void push(char*);
void update(char* projectName);
void upgrade(char* projectName);

/*TODO:
  -I probably created some helper methods that do not need to be used
    -If so, delete them(ONLY AFTER WE COMPLETE ALL FUNCTIONALITY, we may need them in some of the functions we haven't written yet)
*/

int main(int argc, char** argv){

  //Check command line arguments before commiting any runtime
  checkArgs(argc, argv);

  if(strcmp(argv[1], "configure") == 0){
    printf("\nconfigure with IP address %s and port number %s\n", argv[2], argv[3]);
    configure(argv[2], argv[3]);
    return 0;
  }else if(strcmp(argv[1], "checkout") == 0){
    printf("\ncheckout project %s", argv[2]);
    checkout(argv[2]);
  }else if(strcmp(argv[1], "update") == 0){
    printf("\nupdate project %s\n", argv[2]);
    update(argv[2]);
  }else if(strcmp(argv[1], "upgrade") == 0){
    printf("\nupgrade project %s\n", argv[2]);
    upgrade(argv[2]);
  }else if(strcmp(argv[1], "commit") == 0){
    printf("\ncommit project %s\n", argv[2]);
    commit(argv[2]);
  }else if(strcmp(argv[1], "push") == 0){
    printf("\npush project %s\n", argv[2]);
    push(argv[2]);
  }else if(strcmp(argv[1], "create") == 0){
    printf("\ncreate project %s\n", argv[2]);
    create(argv[2]);
  }else if(strcmp(argv[1], "destroy") == 0){
    printf("\ndestroy project %s\n", argv[2]);
    destroy(argv[2]);
  }else if(strcmp(argv[1], "add") == 0){
    printf("\nadd %s to project %s\n", argv[3], argv[2]);
    add(argv[2], argv[3]);
  }else if(strcmp(argv[1], "remove") == 0){
    printf("\nremove file %s from project %s\n", argv[2], argv[3]);
    remove_file(argv[2], argv[3]);
  }else if(strcmp(argv[1], "currentversion") == 0){
    printf("\ncurrent version of project %s\n", argv[2]);
    currentVersion(argv[2]);
  }else if(strcmp(argv[1], "history") == 0){
    printf("\nhistory of project %s\n", argv[2]);
  }else if(strcmp(argv[1], "rollback") == 0){
    printf("\nrollback project %s to version %s\n", argv[2], argv[3]);
  }

  return 0;
}




//Method to verify that the command line arguments are valid
void checkArgs(int argc, char** argv){

  //Checking if there is an appropiate number of command line arguments
  if(argc < 3){
    printf("\nError: too few command line arguments. Please double check your arguments before running again.\n");
    exit(1);
  }
  if(argc > 4){
    printf("\nError: too many command line arguments. Please double check your arguments before running again.\n");
    exit(1);
  }

  //Checking to make sure a valid command is specified
  if(strcmp(argv[1], "push") != 0 && strcmp(argv[1], "create") != 0 && strcmp(argv[1], "destroy") != 0 && strcmp(argv[1], "currentversion") != 0 && strcmp(argv[1], "history") != 0 && strcmp(argv[1], "rollback") != 0 && strcmp(argv[1], "remove") != 0 && strcmp(argv[1], "add") != 0 && strcmp(argv[1], "configure") != 0 && strcmp(argv[1], "checkout") != 0 && strcmp(argv[1], "update") != 0 && strcmp(argv[1], "upgrade") != 0 && strcmp(argv[1], "commit") != 0) {
    printf("\nError: %s is not a valid command. Please make sure you specify a command from the following list:\n\t- push\n\t- create\n\t- destroy\n\t- add\n\t- remove\n\t- currentversion\n\t- history\n\t- rollback\n\t- configure\n\t- checkout\n\t- update\n\t- upgrade\n\t- commit\n", argv[1]);
    exit(1);
  }

  //Checking if there is an appropriate number of command line arguments depending on the command specified
  if((strcmp(argv[1], "push") == 0 || strcmp(argv[1], "create") == 0 || strcmp(argv[1], "destroy") == 0 || strcmp(argv[1], "currentversion") == 0 || strcmp(argv[1], "history") == 0 || strcmp(argv[1], "checkout") == 0 || strcmp(argv[1], "update") == 0 || strcmp(argv[1], "upgrade") == 0 || strcmp(argv[1], "commit") == 0) && argc > 3){
    printf("\nError: too many command line arguments to run %s. Please double check your arguments before running again.\n", argv[1]);
    exit(1);
  }
  if((strcmp(argv[1], "push") == 0 || strcmp(argv[1], "create") == 0 || strcmp(argv[1], "destroy") == 0 || strcmp(argv[1], "currentversion") == 0 || strcmp(argv[1], "history") == 0 || strcmp(argv[1], "checkout") == 0 || strcmp(argv[1], "update") == 0 || strcmp(argv[1], "upgrade") == 0 || strcmp(argv[1], "commit") == 0) && argc < 3){
    printf("\nError: too few command line arguments to run %s. Please double check your arguments before running again.\n", argv[1]);
    exit(1);
  }

  if((strcmp(argv[1], "rollback") == 0 || strcmp(argv[1], "remove") == 0 || strcmp(argv[1], "add") == 0 || strcmp(argv[1], "configure") == 0 ) && argc > 4){
    printf("\nError: too many command line arguments to run %s. Please double check your arguments before running again.\n", argv[1]);
    exit(1);
  }
  if((strcmp(argv[1], "rollback") == 0 || strcmp(argv[1], "remove") == 0 || strcmp(argv[1], "add") == 0 || strcmp(argv[1], "configure") == 0) && argc < 4){
    printf("\nError: too few command line arguments to run %s. Please double check your arguments before running again.\n", argv[1]);
    exit(1);
  }


}

/* Method to handle the configure command
-------------------------------------------------------------
This method takes a hostname/IP address and a port number and stores them in a .configure
file so that the given configuration can be accessed later if the client program is executed
again. This way, if the server is launched with a new port number, the client program can remain
dynamic and the user can specify the new location of the server at any given moment.
*/
void configure(char* host, char* port){
  int configure_fd = open(".configure", O_WRONLY | O_TRUNC | O_CREAT, 0777);

  if(configure_fd < 0){
    printf("\nError: Was unable to open/create the .configure file. Please try again.\n");
    exit(1);
  }
  int initial_size = strlen(host) + strlen(port) + 1;
  char to_be_written[initial_size];
  memset(to_be_written, 0, sizeof to_be_written);
  strcat(to_be_written, host);
  strcat(to_be_written, " ");
  strcat(to_be_written, port);
  int write_size = write(configure_fd, to_be_written, initial_size);
  if(write_size < 0){
    printf("\nError: Was unable to write to the .configure file. Please try again.\n");
    exit(1);
  }

}

/* Function to handle the create command
----------------------------------------
If the create command is given, the client will contact the server
to create a respository for the project on the server side. It will create
a repository for the project on the client side if it doesn't already
exist. It will also recieve a .Manifest file and place it into
the project on the client side.
*/
void create(char* newProject){
  //Prepare message to server
  int i;
  char to_be_written[strlen(newProject) + 3];
  memset(to_be_written, 0, sizeof(to_be_written));
  to_be_written[0] = 'c';
  to_be_written[1] = ' ';
  int message_idx = 2;
  for(i = 0; i < strlen(newProject); i++){
    if(newProject[i]){
      to_be_written[message_idx] = newProject[i];
      message_idx++;
    }
  }
  printf("\nWritten to server: -%s- Size: %d passed in: -%s-\n", to_be_written, sizeof(to_be_written), newProject);

  //Connect to server, store the response
  char* response = connect_to_WTFserver(to_be_written);

  //Report if server responded with an error
  /*
    TODO: Make a method for receiving and alerting user of errors
      -Basically, if the server response begins with a certain character or some indicator of an error, it will send the server response to that method which will print
        the error message out to the user, without including the indicator we decide to use.
  */
  if(strcmp(response, "x") == 0){
    printf("\nError: That project already exists. You can download the current version of the project that is on the server with the checkout command.\n");
    exit(1);
  }else if(strcmp(response, "e") == 0){
    printf("\nError: The server ran into an error processing your request. Please try again.\n");
    exit(1);
  }

  //create repository if it doesn't already exist
  char* command =malloc(sizeof(newProject+7));
  DIR* dir = opendir(newProject);

  //the project exists on the client side do nothing
  if (dir){
    return;
  //Project does not exist so we create it
  }else if (ENOENT == errno){
    strcpy(command,"mkdir ");
    strcat(command, newProject);
    system(command);
  //opendir() failed for some reason
  }else{
    printf("\nERROR: opendir() failed\n");
    exit(1);
  }

  //Create the .Manifest and place it in the new directory
  //Create the .Manifest file
  int manifest_fd = open(".Manifest", O_WRONLY | O_TRUNC | O_CREAT, 0777);

  if(manifest_fd < 0){
    printf("\nError: Was unable to open/create the .Manifest file. Please try again.\n");
    exit(1);
  }

  int write_size = write(manifest_fd, response, strlen(response));
  if(write_size < 0){
    printf("\nError: Was unable to write to the .Manifest file. Please try again.\n");
    exit(1);
  }

  char filepath[10 + strlen(newProject)];
  strcpy(filepath, newProject);
  strcat(filepath, "/.Manifest");
  printf("\n%s\n", filepath);
  rename(".Manifest", filepath);

  closedir(dir);
  free(response);
  free(command);
  return;
}

/* Function to handle the destroy command
----------------------------------------
If the destroy command is given, the client will contact the server
and send it the name of a project that the server will then delete.
If the client cannot connect or the server does not contain the
specified project, destroy will fail.
*/
void destroy(char* projectName){
  //Prepare message to server
  int i;
  char to_be_written[strlen(projectName) + 3];
  memset(to_be_written, 0, sizeof(to_be_written));
  to_be_written[0] = 'd';
  to_be_written[1] = ' ';
  int message_idx = 2;
  for(i = 0; i < strlen(projectName); i++){
    if(projectName[i]){
      to_be_written[message_idx] = projectName[i];
      message_idx++;
    }
  }
  printf("\nWritten to server: -%s- Size: %d passed in: -%s-\n", to_be_written, sizeof(to_be_written), projectName);

  //Connect to server, store the response
  char* response = connect_to_WTFserver(to_be_written);

  //Report if server responded with an error
  /*
    TODO: Make a method for receiving and alerting user of errors
      -Basically, if the server response begins with a certain character or some indicator of an error, it will send the server response to that method which will print
        the error message out to the user, without including the indicator we decide to use.
  */
  if(strcmp(response, "x") == 0){
    printf("\nError: The project %s does not exist on the server.\n", projectName);
    exit(1);
  }else if(strcmp(response, "e") == 0){
    printf("\nError: The server ran into an error processing your request. Please try again.\n");
    exit(1);
  }else{
    printf("\n%s\n", response);
  }
  return;

}


/*Function to handle the add command
----------------------------------------
If the add command is given, the client will first add the file
to its project manifest. The client will then contact the server,
sending over the file for the server to input into its manifest,
as well as duplicate on the server side project folder.
*/
void add(char* project_name, char* file_name){
  //Verify the project exists on the client side
  DIR* dir = opendir(project_name);
  if(!dir){
    printf("\nError: the project you specified does not seem to exist on the client side. Please create the project before running again.\n");
    exit(1);
  }
  closedir(dir);

  char manifest_path[sizeof(project_name) + 10];
  strcpy(manifest_path, project_name);
  strcat(manifest_path, "/.Manifest");

  char file_path[sizeof(project_name) + sizeof(file_name) + 2];
  memset(file_path, '\0', sizeof(project_name) + sizeof(file_name) + 2);
  strcpy(file_path, project_name);
  strcat(file_path, "/");
  strcat(file_path, file_name);

  //Open file to copy and manifest to copy to
  int manifest_fd = open(manifest_path, O_WRONLY | O_APPEND);
  if(manifest_fd < 0){
    printf("\nError: could not open .Manifest. Please try again.\n");
    exit(1);
  }

  int file_fd = open(file_path, O_RDONLY);
  if(file_fd < 0){
    printf("\nError: could not find %s in your project. Make sure it is in the project, and provide a path through any subdirectories when you specify the file's name. Please  Please try again.\n");
    exit(1);
  }

  //Read file contents to a single char*
  char* file_contents = readFile(file_path);
  printf("\n%s\n", file_contents);

  //Hash the file
  char* hashed_file = hashIt(file_contents);

  //Create an entry in the manifest for the new file
  int size_of_write = strlen(project_name) + strlen(hashed_file) + strlen(file_name) + 12;
  char to_be_written[size_of_write];
  strcpy(to_be_written, "A./");
  strcat(to_be_written, file_path);
  strcat(to_be_written, "~V:0~H:");
  strcat(to_be_written, hashed_file);
  strcat(to_be_written, "\n");


  int bytes_written = write(manifest_fd, to_be_written, size_of_write);
  if(bytes_written < 0){
    printf("\nError: was unable to write your file data to the .Manifest file.\n");
    exit(1);
  }

  free(hashed_file);
  free(file_contents);
  close(manifest_fd);
  close(file_fd);
}



/*Helper method to handle the remove command
----------------------------------------
If the remove command is given, the client will remove
the specified file from the local project's manifest.
*/

/*
  TODO: Make this file write the manifest's original content back to it if a writing error occurs
  */
void remove_file(char* project_name, char* file_name){
 //Verify the project exists on the client side
  DIR* dir = opendir(project_name);
  if(!dir){
    printf("\nError: the project you specified does not seem to exist on the client side. Please create the project before running again.\n");
    exit(1);
  }
  closedir(dir);

  //Generate the path of the manifest file
  char manifest_path[sizeof(project_name) + 10];
  strcpy(manifest_path, project_name);
  strcat(manifest_path, "/.Manifest");

  //read manifest file to a single char*
  char* manifest_contents = readFile(manifest_path);
  printf("\n%s\n", manifest_contents);

  //open manifest fd for writing
  int manifest_fd = open(manifest_path, O_WRONLY | O_TRUNC);
  if(manifest_fd < 0){
    printf("\nError: could not open .Manifest. Please try again.\n");
    exit(1);
  }

  //tokenize the manifest file by every line
  char* token;
  char* saveptr = manifest_contents;
  int bytes_written;

  //loop through the tokens
  while(token = strtok_r(saveptr, "\n", &saveptr)){
    printf("\n-%s-", token);
    //if the file name is not found in this line of the manifest, write it back to the manifest
    if(strstr(token, file_name) == NULL){
      bytes_written = write(manifest_fd, token, strlen(token));
      if(bytes_written < 0){
        printf("\nError: was unable to edit .Manifest. Will attempt to write the original information back into it.\n");
        exit(1);
      }
      bytes_written = write(manifest_fd, "\n", 1);
      if(bytes_written < 0){
        printf("\nError: was unable to edit .Manifest. Will attempt to write the original information back into it.\n");
        exit(1);
      }
    }
    //if the filename is found in this line of the manifest, write it back to the manifest with an R in front
    else {
      //write the R
      int bytes_written = write(manifest_fd, "D", 1);
      if(bytes_written < 0){
        printf("\nError: was unable to edit .Manifest. Will attempt to write the original information back into it.\n");
        exit(1);
      }
      //write the rest of the line back
      bytes_written = write(manifest_fd, token, strlen(token));
      if(bytes_written < 0){
        printf("\nError: was unable to edit .Manifest. Will attempt to write the original information back into it.\n");
        exit(1);
      }
      bytes_written = write(manifest_fd, "\n", 1);
      if(bytes_written < 0){
        printf("\nError: was unable to edit .Manifest. Will attempt to write the original information back into it.\n");
        exit(1);
      }
    }

  }
  free(manifest_contents);
  close(manifest_fd);
}

/*Helper method to handle the checkout command
----------------------------------------------
The checkout command will be responsible for receiving
the .Manifest from a project on the server, as well as all
files and sub directories and creating it locally.
*/
void checkout(char* project_name){
  DIR* dir = opendir(project_name);
  if(dir){
    printf("\nError: the project name you specified already exists on the client side.\n");
    return;
  }

  //Prepare message to server
  int i;
  char to_be_written[strlen(project_name) + 3];
  memset(to_be_written, 0, sizeof(to_be_written));
  to_be_written[0] = 'h';
  to_be_written[1] = ' ';
  int message_idx = 2;
  for(i = 0; i < strlen(project_name); i++){
    if(project_name[i]){
      to_be_written[message_idx] = project_name[i];
      message_idx++;
    }
  }
  printf("\nWritten to server: -%s- Size: %d passed in: -%s-\n", to_be_written, sizeof(to_be_written), project_name);

  //Connect to server, store the response
  char* response = connect_to_WTFserver(to_be_written);
  if(strcmp(response, "x") == 0){
    printf("\nError: the project you specified does not exist on the server.\n");
    return;
  }else if(strcmp(response, "e") == 0){
    printf("\nError: the server failed to open the project you specified. Please try again.\n");
    return;
  }

  //Prepare for tokenizing string and storing results
  char* token;
  char* saveptr = response;



  //Isolate the number of files
  token = strtok_r(saveptr, "$", &saveptr);
  int num_files = atoi(token);

  int* file_sizes = malloc(num_files * sizeof(int));
  int* file_name_sizes = malloc(num_files * sizeof(int));
  char** file_names = malloc(num_files * sizeof(char*));
  char** file_contents = malloc(num_files * sizeof(char*));

  int index = 0;

  //loop through the rest of the tokens
  while(token = strtok_r(saveptr, "$", &saveptr)){
    //for each token, take the specific pieces of information from each token
    char* inner_token;
    char* inner_saveptr = token;
    int data_type = 0;
    while(inner_token = strtok_r(inner_saveptr, "&", &inner_saveptr)){
      if(data_type == 0){
        file_name_sizes[index] = atoi(inner_token);
      }else if(data_type == 1){
        file_names[index] = malloc(file_name_sizes[index] * sizeof(char));
        strcpy(file_names[index], inner_token);
      }else if(data_type == 2){
        file_sizes[index] = atoi(inner_token);
      }else{
        file_contents[index] = malloc(file_sizes[index] * sizeof(char));
        strcpy(file_contents[index], inner_token);
      }
      data_type++;
    }
    index++;
  }

  //Tokenize each file name to open any necessary sub directories and create necessary files
  for(i = 0; i < num_files; i++){
    //To store the file path at a given moment
    char file_path_so_far[file_name_sizes[i]];

    //To tokenize the file path
    char* file_name_token;
    char* file_name_saveptr = file_names[i];

    //Get the current directory from path, add it to the path so far
    file_name_token = strtok_r(file_name_saveptr, "/", &file_name_saveptr);
    strcpy(file_path_so_far, file_name_token);


    //Loop through rest of file path, creating directories as needed
    while(file_name_token = strtok_r(file_name_saveptr, "/", &file_name_saveptr)){

      //if it has a '.' in it, it must not be a directory since we removed the current directory from the list of tokens already
      if(strchr(file_name_token, '.') != NULL){
        strcat(file_path_so_far, "/");
        strcat(file_path_so_far, file_name_token);
        int file_fd = open(file_path_so_far, O_WRONLY | O_TRUNC | O_CREAT, 0777);
        if(file_fd < 0){
          printf("\nError: was unable to create file %s. Will skip and continue replicating the rest of the project.\n");
        }else{
          int bytes_written = write(file_fd, file_contents[i], file_sizes[i]);
        }
        close(file_fd);
      //If its not a file, it must be a directory. Create it if the directory does not already exist, and add the directory's name to the current file path
      }else{
          strcat(file_path_so_far, "/");
          strcat(file_path_so_far, file_name_token);
          DIR* dir = opendir(project_name);
          if(!dir){
            int result = mkdir(file_path_so_far, 0777);
            if(result < 0){
              printf("\nError: could not create directory %s. Will skip continue replicating the rest of the project.\n");
            }
          }
          closedir(dir);
      }
    }

  }

  printf("\nStatus: Project %s has been cloned locally.\n", project_name);


  free(file_sizes);
  free(file_name_sizes);
  for(i = 0; i < num_files; i ++){
    free(file_names[i]);
    free(file_contents[i]);
  }
  free(file_names);
  free(file_contents);
}



/* Helper method to handle connecting to the WTFserver
----------------------------------------
This method will be run any time the client must connect
to the server. It will look for a .configure file, and
assuming that file exists, will attempt to connect to the
server using the provided configuration. Assuming this is
successful, it will return a file descriptor for communicating
with the server.
*/
char* connect_to_WTFserver(char* message_to_server){
  //Checking for .configure file, throwing an error if it does not exist
  int config_fd = open(".configure", O_RDONLY);
  if(config_fd < 0){
    printf("\nError: Could not open the .configure file. Please run the configure command before trying again.\n");
    exit(1);
  }

  //Read address and port number from file
  int i;
  int j = 0;
  char config_buffer[1024];
  char address[1020];
  char port[5];
  int bytes_read = read(config_fd, config_buffer, 1024);

  for(i = 0; i < bytes_read; i++){
    if(config_buffer[i] == ' '){
      break;
    }
    address[i] = config_buffer[i];
  }

  address[i] = '\0';

  for(i += 1; i < bytes_read; i++){
    port[j] = config_buffer[i];
    j++;
  }

  port[4] = '\0';

  //Creating the structs/variables neccesary to connect to server
  int sockfd = 0,n = 0;
  struct sockaddr_in serv_addr;

  //Create the socket
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0){
    printf("\n Error: Could not create socket \n");
    exit(1);
  }

  //Set connection configurations
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(port));
  serv_addr.sin_addr.s_addr = inet_addr(address);

  //Attempting to connect to server, retrying every three seconds
  while(1){
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){
      printf("\n Error: Could not find server. Will attempt to recconect in 3 seconds.\n");
      sleep(3);
    }else{
      printf("\nSTATUS: Successfully connected to server.\n");
      break;
    }
  }

  //Prepare buffer for communicating with server
  char* recvBuff = (char*)malloc(1024);
  memset(recvBuff, 0, sizeof(recvBuff));

  //Write to server
  n = write(sockfd, message_to_server, strlen(message_to_server));

  //Read from server, return its response
  while((n = read(sockfd, recvBuff, 1023)) > 0){
    recvBuff[n] = 0;
  }

  return recvBuff;
}

/* Helper method for hashing a file
-------------------------------------------------------
This method takes the contents of a file stored in a char*
as an input and returns the hash of the file as a char*.
*/
char* hashIt(char* input){
  unsigned char digest[MD5_DIGEST_LENGTH];
  MD5(input, strlen(input), digest);
  int i;
  char * result=(char*)malloc(34*sizeof(char));
  memset(result,'\0',34);
  for(i = 0; i < MD5_DIGEST_LENGTH; i++){
     sprintf(result+2*i, "%02x", digest[i]);
  }

  return result;
}

/*Helper method for copying one file to another
-------------------------------------------------------
This is achieved by reading one file into a char* then
writing that char* to another file. This will assist in
communicating files between server and client
*/

/*
    TODO: turn this into a library that both client and server import
*/

void copyFile(char* from_file_name, int to_fd){
  char* from_file_contents = readFile(from_file_name);

  int bytes_written = write(to_fd, from_file_contents, strlen(from_file_contents));
  if(bytes_written < 0){
    printf("\nError: unable to execute the write() function. Please try again.\n");
    exit(1);
  }

  free(from_file_contents);
}

/*Helper method to read a file into a char*
-------------------------------------------------------
This is will help with hashing an entire file, and can be implemented
in the copyFile function
Note: must take the file's name and not a file descriptor
so that we can obtain the file's size using a system call
*/

/*
  TODO: VERY IMPORTANT!!!!!
    -This method will only work for SMALL files
    -Must store an entire file's content in a linked list of strings, or think of a smarter way to do it
  */
char* readFile(char* fileName){
  //Finding file's size
  struct stat st;
  stat(fileName, &st);
  int size = st.st_size;

  char* file_contents = malloc(size * sizeof(char));
  if(!file_contents){
    printf("\nError: was unable to use malloc(). Please try again.\n");
    exit(1);
  }
  memset(file_contents, 0, size);

  int file_fd = open(fileName, O_RDONLY);
  if(file_fd < 0){
    printf("\nError: Ran into trouble reading file %s.", fileName);
    return "Error";
  }

  int bytes_read = read(file_fd, file_contents, size);
  file_contents[bytes_read] = '\0';
  close(file_fd);
  return file_contents;

}

/*Helper method to read a single line of a file to a char*
-------------------------------------------------------
Useful for parsing through the .Manifest file, where everything
is stored in its own line. Essentially reads the file to a char* until
it hits a \n character. Can then be used over and over again
because the file descriptor will continue reading where it left off.
Note: it is much more efficient to read a file into a buffer and copy
that buffer character by character vs. reading the file character
by character. However, trying to copy chunks of the file to a buffer
and stop reading exactly when a \n character is read would be impossible.
The read_line function must work like this so that we can loop through a file,
calling this method repeatedly to get each line of the file.
*/
char* read_line(int fd){
  //Creating a buffer to hold the entire file
  int file_storage_size = 10;
  int file_idx = 0;
  char* file_contents = malloc(file_storage_size * sizeof(char));
  if(!file_contents){
    printf("\nError: was unable to use malloc(). Please try again.\n");
    exit(1);
  }

  //buffer to read character of the file to
  char buffer[1];


  //Copy from file to buffer until \n is reached
  while((read(fd, buffer, 1)) == 1){
    if(buffer[0] == '\n' || buffer[0] == 0 || buffer[0] == EOF){
      break;
    }else{
      file_contents[file_idx] = buffer[0];
      file_idx++;
       if(file_idx == file_storage_size){
        char* larger_file = realloc(file_contents, 2 * file_storage_size * sizeof(char));

        if(!larger_file){
          printf("\nError: was unable to use realloc(). Please try again.\n");
          exit(1);
        }

        file_contents = larger_file;
      }

    }


  }
  //Eliminate any unwanted characters from the realloc
  char* fixed_file_contents = malloc(file_idx * sizeof(char));
  strcpy(fixed_file_contents, file_contents);
  fixed_file_contents[file_idx] = '\0';

  //free the original file_contents
  free(file_contents);

  return fixed_file_contents;
}

void currentVersion(char* projectName){
  //Prepare message to server
  int i;
  char to_be_written[strlen(projectName) + 3];
  memset(to_be_written, 0, sizeof(to_be_written));
  to_be_written[0] = 'v';
  to_be_written[1] = ' ';
  int message_idx = 2;
  for(i = 0; i < strlen(projectName); i++){
    if(projectName[i]){
      to_be_written[message_idx] = projectName[i];
      message_idx++;
    }
  }
  printf("\nWritten to server: -%s- Size: %d passed in: -%s-\n", to_be_written, sizeof(to_be_written), projectName);

  //Connect to server, store the response
  char* response = connect_to_WTFserver(to_be_written);

  //Report if server responded with an error
  /*
    TODO: Make a method for receiving and alerting user of errors
      -Basically, if the server response begins with a certain character or some indicator of an error, it will send the server response to that method which will print
        the error message out to the user, without including the indicator we decide to use.
  */
  if(strcmp(response, "x") == 0){
    printf("\nError: The specified project does not exist on the server. Please check the project name and try again.\n");
    exit(1);
  }else if(strcmp(response, "e") == 0){
    printf("\nError: The server ran into an error processing your request. Please try again.\n");
    exit(1);
  }

  printf("Here is the current version of the project and its contained files:\n");
  printf("%s\n",response);
  free(response);

  return;
}


/*Function to handle the commit command
---------------------------------------
When the user wants to make a commit, the user first downloads the server's
manifest file. The user then compares its local manifest to the server's manifest
line by line. Assuming the user and server have different project versions,
the client checks for local additions, local removals, and local modifications.
Assuming these are the only differences between the local manifest and the server's,
the user tehn sends its commit file to the server.
*/
void commit(char* project_name){
  //Check if project exists
  DIR* dir = opendir(project_name);
  if(!dir){
    printf("\nError: project does not exist locally. Please create it first.\n");
    exit(1);
  }
  closedir(dir);

  //Check for if the local project has a .Update file
  char update_path[strlen(project_name) + 10];
  strcpy(update_path, "./");
  strcat(update_path, project_name);
  strcat(update_path, "/.Update");
  struct stat update_file;
  if(stat(update_path, &update_file) == 0){
    if(update_file.st_size > 0){
      printf("\nError: your local version of the project has a .Update file that is not empty.\n");
      exit(1);
    }
  }

  //Check for if the local project has a .Conflict file
  char conflict_path[strlen(project_name) + 12];
  strcpy(conflict_path, "./");
  strcat(conflict_path, project_name);
  strcat(conflict_path, "/.Conflict");
  struct stat conflict_file;
  if(stat(conflict_path, &conflict_file) == 0){
    printf("\nError: your local version of the project has a .Conflict file.\n");
    exit(1);
  }

  //Prepare message to server
  int i;
  char to_be_written[strlen(project_name) + 3];
  memset(to_be_written, 0, sizeof(to_be_written));
  to_be_written[0] = 'o';
  to_be_written[1] = ' ';
  int message_idx = 2;
  for(i = 0; i < strlen(project_name); i++){
    if(project_name[i]){
      to_be_written[message_idx] = project_name[i];
      message_idx++;
    }
  }

  //Connect to server, store the response
  char* response = connect_to_WTFserver(to_be_written);

  if(strcmp(response, "e") == 0){
    printf("\nError: project does not exist on server. Make sure you create it first.\n");
    exit(1);
  }else if(strcmp(response, "x") == 0){
    printf("\nError: could not find the manifest file on the server.\n");
    exit(1);
  }

  //Open and tokenize local manifest file
  char manifestPath[sizeof(project_name) + 11];
  strcpy(manifestPath, project_name);
  strcat(manifestPath, "/.Manifest");
  char* local_manifest = readFile(manifestPath);

  //To tokenize the local manifest file by \n
  char* local_token;
  char* l_saveptr = local_manifest;

  //Get the first line of both manifest files and compare them, ultimately checking version numbers
  local_token = strtok_r(l_saveptr, "\n", &l_saveptr);
  for(i = 0; i < strlen(local_token); i++){
    if(local_token[i] != response[i]){
      printf("\nError: the local and server project versions do not match. Please update your local version before running again.\n");
      free(response);
      free(local_manifest);
      exit(1);
    }
  }

  //Creating a path for the commit file
  char commitPath[sizeof(project_name) + 8];
  strcpy(commitPath, project_name);
  strcat(commitPath, "/.Commit");


  int commit_fd = open(commitPath, O_WRONLY | O_TRUNC | O_CREAT, 0777);
  if(commit_fd < 0){
    printf("\nError: could not open/create the .Commit file.");
    close(commit_fd);
    free(response);
    free(local_manifest);
    exit(1);
  }

  //Write project name to commit file
  int first_write = write(commit_fd, project_name, strlen(project_name));
  if(first_write < 0){
    printf("\nError: was unable to write to .Commit file.\n");
    close(commit_fd);
    exit(1);
  }

  first_write = write(commit_fd, "\n", 1);
  if(first_write < 0){
    printf("\nError: was unable to write to .Commit file.\n");
    close(commit_fd);
    exit(1);
  }

  //Loop through local manifest, looking for where files were added, removed or modified
  while(local_token = strtok_r(l_saveptr, "\n", &l_saveptr)){

    //If a file was added locally but not on the server yet*********
    if(local_token[0] == 'A'){

      //Tokenize this line of the local manifest to make it easier to identify the file path and its hash
      char* token;
      char* saveptr = local_token;
      token = strtok_r(saveptr, "~", &saveptr);


      //Write A<filepath> to .Commit
      int bytes_written = write(commit_fd, token, strlen(token));
      if(bytes_written < 0){
        printf("\nError: was unable to write to .Commit file.\n");
        close(commit_fd);
        exit(1);
      }


      //Navigate to the file's hash, which we can do manually since the layout of the manifest file is standardized
      token = strtok_r(saveptr, "~", &saveptr);
      token = strtok_r(saveptr, "~", &saveptr);
      char* inner_token;
      char* inner_saveptr = token;
      inner_token = strtok_r(inner_saveptr, ":", &inner_saveptr);
      inner_token = strtok_r(inner_saveptr, ":", &inner_saveptr);


      //Write a separator symbol and the file's hash to .Commit
      bytes_written = write(commit_fd, "^", 1);
      if(bytes_written < 0){
        printf("\nError: was unable to write to .Commit file.\n");
        close(commit_fd);
        exit(1);
      }
      bytes_written = write(commit_fd, inner_token, strlen(inner_token));
      if(bytes_written < 0){
        printf("\nError: was unable to write to .Commit file.\n");
        close(commit_fd);
        exit(1);
      }
      bytes_written = write(commit_fd, "\n", 1);
      if(bytes_written < 0){
        printf("\nError: was unable to write to .Commit file.\n");
        close(commit_fd);
        exit(1);
      }



      //If a file was removed locally but not on the server yet*********
    }else if(local_token[0] == 'D'){
      //Tokenize this line of the local manifest to make it easier to identify the file path and its hash
      char* token;
      char* saveptr = local_token;
      token = strtok_r(saveptr, "~", &saveptr);


      //Write R<filepath> to .Commit
      int bytes_written = write(commit_fd, token, strlen(token));
      if(bytes_written < 0){
        printf("\nError: was unable to write to .Commit file.\n");
        close(commit_fd);
        exit(1);
      }


      //Navigate to the file's hash, which we can do manually since the layout of the manifest file is standardized
      token = strtok_r(saveptr, "~", &saveptr);
      token = strtok_r(saveptr, "~", &saveptr);
      char* inner_token;
      char* inner_saveptr = token;
      inner_token = strtok_r(inner_saveptr, ":", &inner_saveptr);
      inner_token = strtok_r(inner_saveptr, ":", &inner_saveptr);


      //Write a separator symbol and the file's hash to .Commit
      bytes_written = write(commit_fd, "^", 1);
      if(bytes_written < 0){
        printf("\nError: was unable to write to .Commit file.\n");
        close(commit_fd);
        exit(1);
      }
      bytes_written = write(commit_fd, inner_token, strlen(inner_token));
      if(bytes_written < 0){
        printf("\nError: was unable to write to .Commit file.\n");
        close(commit_fd);
        exit(1);
      }
      bytes_written = write(commit_fd, "\n", 1);
      if(bytes_written < 0){
        printf("\nError: was unable to write to .Commit file.\n");
        close(commit_fd);
        exit(1);
      }

      //If a file has not been added or removed on local manifest (checking for modifications)*********
    }else{
      //Tokenize this line of the local manifest to make it easier to identify the file path and its hash
      char* token;
      char* saveptr = local_token;
      token = strtok_r(saveptr, "~", &saveptr);

      //Preserve server response in temporary buffer so that strtok does not mess it up
      char* temp_buff = (char*)malloc(strlen(response) + 1);
      if(temp_buff == NULL){
        printf("\nError: could not use malloc().\n");
        exit(1);
      }
      strcpy(temp_buff, response);


      //Get a pointer to this file in the server's manifest
      char* file_in_server = strstr(temp_buff, token);
      if(!file_in_server){
        printf("\nError: commit failed. Found a local file that was not recently added that could not be found on the server.\n");
        char command[strlen(commitPath) + 7];
        strcpy(command, "rm -rf ");
        strcat(command, commitPath);
        system(command);
        exit(1);
      }

      //Storing the path for the file so we can hash it later if we need to
      char* buff_for_file_path = malloc(strlen(token) + 1);
      strcpy(buff_for_file_path, token);



      //Navigate to both the server and the client's hash
      token = strtok_r(saveptr, "~", &saveptr);
      char* v_num_storage = malloc(strlen(token) + 1);
      strcpy(v_num_storage, token);
      token = strtok_r(saveptr, "~", &saveptr);
      char* l_inner_token;
      char* l_inner_saveptr = token;
      l_inner_token = strtok_r(l_inner_saveptr, ":", &l_inner_saveptr);
      l_inner_token = strtok_r(l_inner_saveptr, ":", &l_inner_saveptr);

      char* temp_token;
      char* t_saveptr = file_in_server;
      temp_token = strtok_r(t_saveptr, "\n", &t_saveptr);
      char* s_token;
      char* s_saveptr = temp_token;
      s_token = strtok_r(s_saveptr, "~", &s_saveptr);
      s_token = strtok_r(s_saveptr, "~", &s_saveptr);
      char* version_number_storage = malloc(strlen(s_token) + 1);
      strcpy(version_number_storage, s_token);
      s_token = strtok_r(s_saveptr, "~", &s_saveptr);
      char* s_inner_token;
      char* s_inner_saveptr = s_token;
      s_inner_token = strtok_r(s_inner_saveptr, ":", &s_inner_saveptr);
      s_inner_token = strtok_r(s_inner_saveptr, ":", &s_inner_saveptr);


      //If they have the same hash
      if(strcmp(l_inner_token, s_inner_token) == 0){
        char* file_contents = readFile(buff_for_file_path);
        char* file_hash= hashIt(file_contents);

        //Extract version numbers
        char* yet_another_token;
        char* yet_another_saveptr = version_number_storage;
        yet_another_token = strtok_r(yet_another_saveptr, ":", &yet_another_saveptr);
        yet_another_token = strtok_r(yet_another_saveptr, ":", &yet_another_saveptr);
        int versionNumber_s = atoi(yet_another_token);

        char* hopefullly_last_token;
        char* hopefully_last_saveptr = v_num_storage;
        hopefullly_last_token = strtok_r(hopefully_last_saveptr, ":", &hopefully_last_saveptr);
        hopefullly_last_token = strtok_r(hopefully_last_saveptr, ":", &hopefully_last_saveptr);
        int versionNumber_l = atoi(hopefullly_last_token);


        //If the file's live has is  different than the one on the local manifest
        if(strcmp(file_hash, l_inner_token) != 0){
          //Increment version number of local copy
          versionNumber_l++;
          char numStorage[5];
          sprintf(numStorage, "%d", versionNumber_l);

          //Add to the .Commit
          int bytes_written = write(commit_fd, "M", 1);
          if(bytes_written < 0){
            printf("\nError: was unable to write to .Commit file.\n");
            close(commit_fd);
            exit(1);
          }
          bytes_written = write(commit_fd, buff_for_file_path, strlen(buff_for_file_path));
          if(bytes_written < 0){
            printf("\nError: was unable to write to .Commit file.\n");
            close(commit_fd);
            exit(1);
          }
          bytes_written = write(commit_fd, "^", 1);
          if(bytes_written < 0){
            printf("\nError: was unable to write to .Commit file.\n");
            close(commit_fd);
            exit(1);
          }
          bytes_written = write(commit_fd, numStorage, strlen(numStorage));
          if(bytes_written < 0){
            printf("\nError: was unable to write to .Commit file.\n");
            close(commit_fd);
            exit(1);
          }
          bytes_written = write(commit_fd, "^", 1);
          if(bytes_written < 0){
            printf("\nError: was unable to write to .Commit file.\n");
            close(commit_fd);
            exit(1);
          }
          bytes_written = write(commit_fd, file_hash, strlen(file_hash));
          if(bytes_written < 0){
            printf("\nError: was unable to write to .Commit file.\n");
            close(commit_fd);
            exit(1);
          }
          bytes_written = write(commit_fd, "\n", 1);
          if(bytes_written < 0){
            printf("\nError: was unable to write to .Commit file.\n");
            close(commit_fd);
            exit(1);
          }

          //different hashes, compare the version numbers to see if client must sync its repository
        }else{
          if(versionNumber_s > versionNumber_l){
            printf("\nError: please synch this project to the server's project before committing changes.");
            char command[strlen(commitPath) + 7];
            strcpy(command, "rm -rf ");
            strcat(command, commitPath);
            system(command);
            exit(1);
          }
        }

        free(file_contents);
        free(file_hash);
      }

      free(version_number_storage);
      free(temp_buff);
      free(buff_for_file_path);
    }

  }
  close(commit_fd);
  struct stat commit;
  stat(commitPath, &commit);
  int size = commit.st_size;
  if(size == 0){
    char command[strlen(commitPath) + 7];
    strcpy(command, "rm -rf ");
    strcat(command, commitPath);
    system(command);
  }else{
    char* commit_contents = readFile(commitPath);
    char sendBuff[strlen(commit_contents) + strlen(project_name) + 3];
    memset(sendBuff, 0, strlen(commit_contents) + strlen(project_name) + 3);
    strcpy(sendBuff, "z ");
    strcat(sendBuff, project_name);
    strcat(sendBuff, " ");
    strcat(sendBuff, commit_contents);
    char* server_response_2 = connect_to_WTFserver(sendBuff);
    printf("\n%s", server_response_2);
    free(commit_contents);
    free(server_response_2);
  }

  printf("\n");

  free(response);
  free(local_manifest);
}


/*Funciton to handle the push command
-------------------------------------------
If the user runs the push command, the user sends its commit file to the server
and await a success or failure message, and regardless of the response,
delete the local commit file.
*/
void push(char* projectName){
  //Creating a path for the commit filec
  char commitPath[sizeof(projectName) + 8];
  strcpy(commitPath, projectName);
  strcat(commitPath, "/.Commit");

  //Checking for its existence
  int commitfd = open(commitPath,  O_RDONLY);
  if(commitfd < 0){
    printf("\nError: was unable to open .Commit file for the project you specified. Please make sure both the project and the .Commit file exist.\n");
    exit(1);
  }
  close(commitfd);

  //Read commit file to a char*
  char* commitContents = readFile(commitPath);

  //Will be storing all file paths in a char**
  int i;
  char** file_paths = malloc(100 * sizeof(char*));
  for(i = 0; i < 100; i++){
    file_paths[i] = malloc(124 * sizeof(char));
  }
  int file_paths_idx = 1;

  //tokenize the commit file by every line
  char* token;
  char* saveptr = commitContents;

  //loop through the tokens
  while(token = strtok_r(saveptr, "\n", &saveptr)){
    //store each file path in the commit
    if(strcmp(token, projectName) == 0){
      continue;
    }
    int idx = 0;
    char file_path[strlen(token)];
    while(token[idx] != '^'){
      file_path[idx] = token[idx];
      idx++;
    }
    file_path[idx] = '\0';
    strcpy(file_paths[file_paths_idx], file_path);
    file_paths_idx++; 
  }

  //Adding .Commit to the list of file paths
  strcpy(file_paths[0], "./");
  strcat(file_paths[0], commitPath);

  //Getting a copy of the .Commit to send to the client
  char* commit = readFile(commitPath);

  //Looping through all file paths, getting their sizes
  int file_sizes[file_paths_idx];
  
  for(i = 0; i < file_paths_idx; i++){
    if(file_paths[i][0] != 'A' && file_paths[i][0] != 'M' && file_paths[i][0] != 'D'){
      char* temp = readFile(file_paths[i]);
      file_sizes[i] = strlen(temp);
      free(temp);
    }else{
      char* temp = readFile(strchr(file_paths[i], '.'));
      
      file_sizes[i] = strlen(temp);
      free(temp);
    }
   
  }

  //Calculate bytes needed for message to client
  int bytes_needed = 0;
  char numStorage[15];
  sprintf(numStorage, "%d", file_paths_idx);
  bytes_needed += strlen(numStorage);
  for(i = 0; i< file_paths_idx; i++){
    //size of file
    bytes_needed += file_sizes[i];
    //length of file path
    bytes_needed += strlen(file_paths[i]);
    //the bytes it will take to write the size of the file's contents
    sprintf(numStorage, "%d %d", file_sizes[i], i);
    bytes_needed += strlen(numStorage);
    //the bytes it will take to write the size of the file's name
    sprintf(numStorage, "%d", strlen(file_paths[i]));
    bytes_needed += strlen(numStorage);
    bytes_needed += 5;
  }
  bytes_needed += strlen(projectName);

  //Set up buffer for responding to client
  char sendBuff[bytes_needed];
  memset(sendBuff, '\0', sizeof(sendBuff));

  //Copy the number of files to sendBuff
  sprintf(numStorage, "%d", file_paths_idx);
  strcpy(sendBuff, "p ");
  strcat(sendBuff, projectName);
  strcat(sendBuff, " ");
  strcat(sendBuff, numStorage);


  //for each file, copy the length of its path, its path, the length of its contents, and its contents
  for(i = 0; i < file_paths_idx; i++){
    strcat(sendBuff, "$");
    sprintf(numStorage, "%d", strlen(file_paths[i]));
    strcat(sendBuff, numStorage);
    strcat(sendBuff, "&");
    strcat(sendBuff, file_paths[i]);
    strcat(sendBuff, "&");
    sprintf(numStorage, "%d", file_sizes[i]);
    strcat(sendBuff, numStorage);
    strcat(sendBuff, "&");
    if(file_paths[i][0] != 'A' && file_paths[i][0] != 'M' && file_paths[i][0] != 'D'){
      char* contents = readFile(file_paths[i]);
      strcat(sendBuff, contents);
    }else{
      char* contents = readFile(strchr(file_paths[i], '.'));
      strcat(sendBuff, contents);
    }
    


  }
  strcat(sendBuff, "\0");

  printf("\nBytes: %d\n %s\n\n", bytes_needed, sendBuff);


  //Send this info to the server, receive its response and print it
  char* response = connect_to_WTFserver(sendBuff);
  printf("\n%s", response);
  char finished[100];
  sprintf(finished, "rm -rf %s", commitPath);
  system(finished);

  //free(response);
  for(i = 0; i < 100; i++){
    free(file_paths[i]);
  }
  free(file_paths);
  free(commitContents);

}

/*Helper method to handle the update command
----------------------------------------------
The update command will be responsible for receiving
the .Manifest from a project on the server, and comparing it with the local Manifest.
It will output if the current manifest is up to date or if it needs to update itself.
*/
void update(char* projectName){
  //Prepare message to server
  int i;
  char to_be_written[strlen(projectName) + 3];
  memset(to_be_written, 0, sizeof(to_be_written));
  to_be_written[0] = 'u';
  to_be_written[1] = ' ';
  int message_idx = 2;
  for(i = 0; i < strlen(projectName); i++){
    if(projectName[i]){
      to_be_written[message_idx] = projectName[i];
      message_idx++;
    }
  }
  printf("\nWritten to server: -%s- Size: %d passed in: -%s-\n", to_be_written, sizeof(to_be_written), projectName);

  //Connect to server, store the response
  char* response = connect_to_WTFserver(to_be_written);

  //Report if server responded with an error
  if(strcmp(response, "x") == 0){
    printf("\nError: That project already exists. You can download the current version of the project that is on the server with the checkout command.\n");
    exit(1);
  }else if(strcmp(response, "e") == 0){
    printf("\nError: The server ran into an error processing your request. Please try again.\n");
    exit(1);
  }
  
  //Verify the project exists on the client side
  DIR* dir = opendir(projectName);
  if(!dir){
    printf("\nError: the project you specified does not seem to exist on the client side. Please create the project before running again.\n");
    exit(1);
  }
  closedir(dir);

  char manifest_path[sizeof(projectName) + 10];
  strcpy(manifest_path, projectName);
  strcat(manifest_path, "/.Manifest");



  //Read client manifest contents to a single char*
  char* clientManifest = readFile(manifest_path);


  
  //initialize for tokenizing clientMan and serverMan by line
  char* clientToken;
  char* serverToken;
  char* serverSaveptr=malloc(strlen(response));
  char* clientSaveptr = malloc(strlen(clientManifest));
  strcpy(serverSaveptr,response);
  strcpy(clientSaveptr,clientManifest);

  //helper variables for comparisons
  //REMEMBER TO FREE UPDATE FILE PATH, CONFLICT FILE PATH, AND COMMAND AS NEEDED
  int update_fd;
  int conflict_fd;
  char* updateFilePath=(char*)malloc(strlen(projectName) + 8);
  strcpy(updateFilePath, projectName);
  strcat(updateFilePath, "/.Update");
  char* conflictFilePath=(char*)malloc(strlen(projectName) + 10);
  strcpy(conflictFilePath, projectName);
  strcat(conflictFilePath, "/.Conflict");
  clientToken=strtok_r(clientSaveptr,"\n",&clientSaveptr);
  serverToken=strtok_r(serverSaveptr,"\n",&serverSaveptr);
  char* command=(char*)malloc(strlen(projectName)+strlen(conflictFilePath)+30);

  //FULL SUCCESS CASE
  //check to see if client and server manifests are same version numbers
  //if so the client is up to date and .update can be empty.
  //create new update and remove old one
  strcpy(command,"rm -rf ");
  strcat(command,updateFilePath);
  system(command);
  command=(char*)malloc(sizeof(projectName)+sizeof(conflictFilePath)+30);
  strcpy(command,"rm -rf ");
  strcat(command,conflictFilePath);
  system(command);
  update_fd = open(updateFilePath, O_WRONLY | O_CREAT, 0777);
  if(strcmp(clientToken,serverToken)==0){
    //remove .Conflict

    printf("Up To Date\n");
    free(updateFilePath);
    free(conflictFilePath);
    close(update_fd);
    return;

  }
  char* serverManCopy=(char*)malloc(strlen(response)+1);
  strcpy(serverManCopy,response);
  char* serverSaveptr2;
  int bytes_written;
  char* serverLineCopy=(char*)malloc(strlen(response));

  //create loop to tokenize server and check for files the client needs to download
  //skip project version line
  serverToken=strtok_r(serverManCopy,"\n",&serverSaveptr2);
  while(serverToken=strtok_r(NULL,"\n",&serverSaveptr2)){
    char* innerServerSaveptr;
    char* innerServerToken;
    strcpy(serverLineCopy,serverToken);
    innerServerToken=strtok_r(serverLineCopy,"~",&innerServerSaveptr);
    //Server file name should be in innerServerToken
    //if it isn't in client manifest, client will be notifed to download
    //in .Update. Else, we continue
    if(strstr(clientManifest,innerServerToken)==NULL){
      strcpy(command,"A");
      strcat(command,serverToken);
      strcat(command,"\n");
      bytes_written = write(update_fd, command, strlen(command));
      if(bytes_written < 0){
        printf("\nError: was unable to edit .Update. Will attempt to write the original information back into it.\n");
        exit(1);
      }
      printf(command);
    }

  }

  //now we must loop through and see if the client has files that the server got rid of
  char* clientManCopy=(char*)malloc(strlen(clientManifest)+1);
  strcpy(clientManCopy,clientManifest);
  char* clientSaveptr2;
  char* clientLineCopy=(char*)malloc(strlen(response));
  
  //skip project version line
  clientToken=strtok_r(clientManCopy,"\n",&clientSaveptr2);
  while(clientToken=strtok_r(NULL,"\n",&clientSaveptr2)){
    char* innerClientSaveptr;
    char* innerClientToken;
    strcpy(clientLineCopy,clientToken);
    innerClientToken=strtok_r(clientLineCopy,"~",&innerClientSaveptr);
    //Server file name should be in innerServerToken
    //if it isn't in client manifest, client will be notifed to download
    //in .Update. Else, we continue
    if(strstr(response,innerClientToken)==NULL){
      strcpy(command,"D");
      strcat(command,clientToken);
      strcat(command,"\n");
      bytes_written = write(update_fd, command, strlen(command));
      if(bytes_written < 0){
        printf("\nError: was unable to edit .Update. Will attempt to write the original information back into it.\n");
        exit(1);
      }
      printf(command);
    }

  }



  //loop for conflict and modify
  strcpy(serverManCopy,response);
  char* serverSaveptr3;
  char* innerClientToken3;
  char* innerServerToken3;
  char* filePath=(char*)malloc(strlen(clientManifest));
  char* liveHash=(char*)malloc(strlen(clientManifest));


  
  serverToken=strtok_r(serverManCopy,"\n",&serverSaveptr3);
  //for every file in server
  while(serverToken=strtok_r(NULL,"\n",&serverSaveptr3)){

    strcpy(clientManCopy,clientManifest);
    char* clientSaveptr3;
    clientToken =strtok_r(clientManCopy,"\n", &clientSaveptr3);

    //for every file in client
    while (clientToken=strtok_r(NULL,"\n", &clientSaveptr3)){
      char* innerServerSaveptr3;
      char* innerClientSaveptr3;
      //tokenize file names for each
      strcpy(clientLineCopy,clientToken);
      strcpy(serverLineCopy,serverToken);
      innerClientToken3=strtok_r(clientLineCopy,"~",&innerClientSaveptr3);
      innerServerToken3=strtok_r(serverLineCopy,"~",&innerServerSaveptr3);
      strcpy(filePath,innerClientToken3);



      //if the files have the same name we got some comparisons to make
      if(strcmp(innerClientToken3,innerServerToken3)==0){
        //now we check if the version numbers are the different
        innerClientToken3=strtok_r(NULL,"~",&innerClientSaveptr3);
        innerServerToken3=strtok_r(NULL,"~",&innerServerSaveptr3);

        //if they are different we must compare the live hash of the file
        //to the hash in the client manifest. If they are the same
        //its a modify. if they are different its a conflict
        if(strcmp(innerServerToken3,innerClientToken3)!=0){
          innerClientToken3=strtok_r(NULL,"~",&innerClientSaveptr3);
          char* fileContents=readFile(filePath);
          //innerClientToken3=strtok_r(NULL,"\n",&innerClientSaveptr3);

          char* fileHash=hashIt(fileContents);
          strcpy(liveHash,"H:");
          strcat(liveHash,fileHash);

          //if fileHashes are the same all it's a modify
          if(strcmp(liveHash,innerClientToken3)==0){
            strcpy(command,"M");
            strcat(command,serverToken);
            strcat(command,"\n");
            bytes_written = write(update_fd, command, strlen(command));
            if(bytes_written < 0){
              printf("\nError: was unable to edit .Update. Will attempt to write the original information back into it.\n");
              exit(1);
            }
            printf(command);
          }

          //else it's a conflict
          else{
            conflict_fd=open(conflictFilePath,O_WRONLY|O_CREAT|O_APPEND,0777);
            strcpy(command,"C");
            strcat(command,serverToken);
            strcat(command,"\n");
            bytes_written = write(conflict_fd, command, strlen(command));
            if(bytes_written < 0){
              printf("\nError: was unable to edit .Conflict. Will attempt to write the original information back into it.\n");
              exit(1);
            }
            printf(command);
            close(conflict_fd);
          }
          
          
        }

      }


    }
    
  }
  free(clientManCopy);
  free(serverManCopy);
  free(clientLineCopy);
  free(serverLineCopy);
  free(filePath);
  free(liveHash);
  free(conflictFilePath);
  free(updateFilePath);
  close(update_fd);
  return;
  
}

/*
Helper method to handle the upgrade command
----------------------------------------------
If there are any pending updates, this method will make them as needed. It will remove
files that must be deleted, and fetch files that must be added and modified from the
server.
*/
void upgrade(char* projectName){
  //check to see if project exists on client side
  //Verify the project exists on the client side
  DIR* dir = opendir(projectName);
  if(!dir){
    printf("\nError: the project you specified does not seem to exist on the client side. Please create the project before running again.\n");
    exit(1);
  }
  closedir(dir);

  //check to see if .Conflict exists
  char* conflictFilePath=(char*)malloc(strlen(projectName) + 10);
  strcpy(conflictFilePath, projectName);
  strcat(conflictFilePath, "/.Conflict");
  int conflict_fd=open(conflictFilePath,O_RDONLY);
  if(!(conflict_fd<0)){
    printf("There is at least one conflict between the server and client. Please resolve, update and try again.\n");
    close(conflict_fd);
    exit(1);
  }
  close(conflict_fd);

  //check to make sure .Update exists
  char* updateFilePath=(char*)malloc(strlen(projectName) + 9);
  strcpy(updateFilePath, projectName);
  strcat(updateFilePath, "/.Update");
  int update_fd=open(updateFilePath,O_RDONLY);
  if(update_fd<0){
    printf(".Update file does not exist. Please call update and try again.\n");
    exit(1);
  }
  close(update_fd);
  //turn .Update to string
  char* update_contents=readFile(updateFilePath);

  //check to see if .Update is empty, in which case this is up to date
  if(strlen(update_contents)==0){
    printf("The project is up to date.\n");
    close(update_fd);
  }
  char* updateCopy=(char*)malloc(sizeof(strlen(update_contents)+1));
  strcpy(updateCopy,update_contents);
  //now we look through lines of .Update looking for delete. This is easiest.


  char* updateToken;
  char* updateSaveptr;
  char* updateLine=(char*)malloc(strlen(update_contents)+1);
  while(updateToken=strtok_r(updateCopy,"\n",&updateSaveptr)){
    //if it's a delete we can take care of it now
    if(updateToken[0]=='D'){
      char* innerUpdateSaveptr;
      strcpy(updateLine,updateToken);


    }

    //modify or delete. copy file path to send contents string.
    else{

    }


  }
  char command[strlen(projectName) + 9];
  strcpy(command,"rm -rf ");
  strcat(command, projectName);
  system(command);
  int i;
  char to_be_written[strlen(projectName) + 3];
  memset(to_be_written, 0, sizeof(to_be_written));
  to_be_written[0] = 'g';
  to_be_written[1] = ' ';
  int message_idx = 2;
  for(i = 0; i < strlen(projectName); i++){
    if(projectName[i]){
      to_be_written[message_idx] = projectName[i];
      message_idx++;
    }
  }
  printf("\nWritten to server: -%s- Size: %d passed in: -%s-\n", to_be_written, sizeof(to_be_written), projectName);

  //Connect to server, store the response
  char* response = connect_to_WTFserver(to_be_written);
  if(strcmp(response, "x") == 0){
    printf("\nError: the project you specified does not exist on the server.\n");
    return;
  }else if(strcmp(response, "e") == 0){
    printf("\nError: the server failed to open the project you specified. Please try again.\n");
    return;
  }

  //Prepare for tokenizing string and storing results
  char* token;
  char* saveptr = response;



  //Isolate the number of files
  token = strtok_r(saveptr, "$", &saveptr);
  int num_files = atoi(token);

  int* file_sizes = malloc(num_files * sizeof(int));
  int* file_name_sizes = malloc(num_files * sizeof(int));
  char** file_names = malloc(num_files * sizeof(char*));
  char** file_contents = malloc(num_files * sizeof(char*));

  int index = 0;

  //loop through the rest of the tokens
  while(token = strtok_r(saveptr, "$", &saveptr)){
    //for each token, take the specific pieces of information from each token
    char* inner_token;
    char* inner_saveptr = token;
    int data_type = 0;
    while(inner_token = strtok_r(inner_saveptr, "&", &inner_saveptr)){
      if(data_type == 0){
        file_name_sizes[index] = atoi(inner_token);
      }else if(data_type == 1){
        file_names[index] = malloc(file_name_sizes[index] * sizeof(char));
        strcpy(file_names[index], inner_token);
      }else if(data_type == 2){
        file_sizes[index] = atoi(inner_token);
      }else{
        file_contents[index] = malloc(file_sizes[index] * sizeof(char));
        strcpy(file_contents[index], inner_token);
      }
      data_type++;
    }
    index++;
  }

  //Tokenize each file name to open any necessary sub directories and create necessary files
  for(i = 0; i < num_files; i++){
    //To store the file path at a given moment
    char file_path_so_far[file_name_sizes[i]];

    //To tokenize the file path
    char* file_name_token;
    char* file_name_saveptr = file_names[i];

    //Get the current directory from path, add it to the path so far
    file_name_token = strtok_r(file_name_saveptr, "/", &file_name_saveptr);
    strcpy(file_path_so_far, file_name_token);


    //Loop through rest of file path, creating directories as needed
    while(file_name_token = strtok_r(file_name_saveptr, "/", &file_name_saveptr)){

      //if it has a '.' in it, it must not be a directory since we removed the current directory from the list of tokens already
      if(strchr(file_name_token, '.') != NULL){
        strcat(file_path_so_far, "/");
        strcat(file_path_so_far, file_name_token);
        int file_fd = open(file_path_so_far, O_WRONLY | O_TRUNC | O_CREAT, 0777);
        if(file_fd < 0){
          printf("\nError: was unable to create file %s. Will skip and continue replicating the rest of the project.\n");
        }else{
          int bytes_written = write(file_fd, file_contents[i], file_sizes[i]);
        }
        close(file_fd);
      //If its not a file, it must be a directory. Create it if the directory does not already exist, and add the directory's name to the current file path  
      }else{
          strcat(file_path_so_far, "/");
          strcat(file_path_so_far, file_name_token);
          DIR* dir = opendir(projectName);
          if(!dir){
            int result = mkdir(file_path_so_far, 0777);
            if(result < 0){
              printf("\nError: could not create directory %s. Will skip continue replicating the rest of the project.\n");
            }
          }
          closedir(dir);
      }
    }

  }


  free(file_sizes);
  free(file_name_sizes);
  for(i = 0; i < num_files; i ++){
    free(file_names[i]);
    free(file_contents[i]);
  }
  free(file_names);
  free(file_contents);

  

  close(update_fd);
  return;
}

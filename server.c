#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>


//Global variables
int server_fd;

//Struct definition
typedef struct pthread_arg_t {
    int connection_fd;
    struct sockaddr_in client_address;
} pthread_arg_t;

//Function headers
void *pthread_function(void *arg);
void signal_handler(int signal_number);
void create(char*, int);
void destroy(char*, int);
char* hashIt(char*);
void currentVersion(char*, int);
char* readFile(char* fileName);
void checkout(char*, int);
void commit(char*, int);
void recv_commit(char*, int);
void push(char*, int);
void update(char*, int);

//Declare a mutex lock so that in the push command we can lock a repository
pthread_mutex_t lock;

int main(int argc, char *argv[]) {

    //Declare variables
    int port_number, connection_fd;
    struct sockaddr_in address;
    pthread_attr_t pthread_attribute;
    pthread_arg_t *pthread_arguments;
    pthread_t pthread;
    socklen_t client_address_size;

    //Get server port
    if(argc != 2){
        perror("\nError: Incorrect amount of command line parameters. Please only specify this file's executable and a port number to listen on.\n");
        exit(1);
    }

    port_number = atoi(argv[1]);

    //Initialize server address
    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port_number);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    //Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("\nError: could not create server socket.\n");
        exit(1);
    }

    //Bind server to socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof address) == -1) {
        perror("\nError: could not bind server socket.\n");
        exit(1);
    }

    //Begin listening on socket
    if (listen(server_fd, 10) < 0) {
        perror("\nError: server was unable to listen.\n");
        exit(1);
    }

    //Signal handler to handle closing the server with ctrl-c
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    //Initialize pthread attribute
    if (pthread_attr_init(&pthread_attribute) != 0) {
        perror("\nError: could not initialize pthread attribute.\n");
        exit(1);
    }
    if (pthread_attr_setdetachstate(&pthread_attribute, PTHREAD_CREATE_DETACHED) != 0) {
        perror("\nError: could not create detatched threads.\n");
        exit(1);
    }

    while (1) {
        //Create pthread instance for each client
        pthread_arguments = (pthread_arg_t *)malloc(sizeof *pthread_arguments);
        if (!pthread_arguments) {
            perror("\nError: Could not malloc for the server thread arguments\n");
            continue;
        }

        //Accept connection to client
        client_address_size = sizeof pthread_arguments->client_address;
        connection_fd = accept(server_fd, (struct sockaddr *)&pthread_arguments->client_address, &client_address_size);
        if (connection_fd == -1) {
            perror("\nError: could not accept connection from client.\n");
            free(pthread_arguments);
            continue;
        }

        printf("\nSTATUS: Accepted connection from client.\n");

        //Initialize pthread argument for the connection file descriptor
        pthread_arguments->connection_fd = connection_fd;

        //Create the thread to handle the client
        if (pthread_create(&pthread, &pthread_attribute, pthread_function, (void *)pthread_arguments) != 0) {
            perror("\nError: could not create a thread to handle the client.\n");
            free(pthread_arguments);
            continue;
        }
    }

    return 0;
}

//Function to handle a client
void *pthread_function(void *arg) {

    //Get values from the pthread argument
    pthread_arg_t *pthread_arguments = (pthread_arg_t *)arg;
    int connection_fd = pthread_arguments->connection_fd;
    free(arg);


    //Create and initialize buffers to read/write from/to client
    char recvBuff[1024];
    char sendBuff[1024];

    memset(sendBuff, 0, sizeof(sendBuff));
    memset(recvBuff, 0, sizeof(recvBuff));

    //Read message from client
    int n =read(connection_fd, recvBuff, sizeof(recvBuff));
    recvBuff[n] = '\0';
    printf("Message from client: -%s- size %d", recvBuff, n);

    //Handles push requests
    if(recvBuff[0] == 'p'){
        printf("Push request recieved\n");
        push(recvBuff, connection_fd);

    //Handles create requests
    }else if (recvBuff[0] == 'c'){
        printf("Create request recieved\n");
        create(recvBuff, connection_fd);


    //Handles destroy requests
    }else if (recvBuff[0] == 'd'){
        printf("Destroy request recieved\n");
        destroy(recvBuff, connection_fd);

    //Handles checkout requests
    }else if (recvBuff[0] == 'h'){
        printf("Checkout request recieved\n");
        checkout(recvBuff, connection_fd);

    //Handles update requests
    }else if (recvBuff[0] == 'u'){
        printf("Update request recieved\n");
        update(recvBuff, connection_fd);

    //Handles upgrade requests
    }else if (recvBuff[0] == 'g'){
        printf("Upgrade request recieved\n");
        checkout(recvBuff, connection_fd);

    //Handles commit requests
    }else if (recvBuff[0] == 'o'){
        printf("Commit request recieved\n");
        commit(recvBuff, connection_fd);

    //Handles currentversion requests
    }else if (recvBuff[0] == 'v'){
        printf("Currentversion request recieved\n");
        currentVersion(recvBuff, connection_fd);

    //Handles history requests
    }else if (recvBuff[0] == 'h'){
        printf("History request recieved\n");
        strcpy(sendBuff, "test2.txt");
        write(connection_fd, sendBuff, strlen(sendBuff));

    //Handles rollback requests
    }else if (recvBuff[0] == 'r'){
        printf("Rollback request recieved\n");
        strcpy(sendBuff, "test2.txt");
        write(connection_fd, sendBuff, strlen(sendBuff));
    }else if(recvBuff[0] == 'z'){
      printf("Receiving a .Commit file\n");
      recv_commit(recvBuff, connection_fd);
    }

    close(connection_fd);
    printf("\nSTATUS: Client disconnected froms server.\n");
    return NULL;
}

//Method to free the server socket file descriptor at exit
void signal_handler(int signal_number) {
    pthread_mutex_destroy(&lock);
    close(server_fd);
    exit(0);
}

/*
Create
----------------------------------------
If the server is sent the create command, it will recieve the name of the
new project from the client and create a directory for the project
as well as a manifest file. The server will then respond to the client with the manifest file.
*/
void create(char* command, int connfd){
  //Parse project name from the message received from the client
  char project_name[strlen(command) - 1];
  memset(project_name, 0, sizeof(project_name));
  int i;
  for(i = 2; i < strlen(command); i++){
      if(command[i]){
        project_name[i - 2] = command[i];
      }

  }

  //Set up buffer for responding to client
  char sendBuff[1024];
  memset(sendBuff, '\0', sizeof(sendBuff));

  //Attempt to open directory with the name specified by the client
  DIR* dir = opendir(project_name);

  //Report an error assuming the project already exists
  if(dir){
    closedir(dir);
    strcpy(sendBuff, "x");
    printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
    write(connfd, sendBuff, strlen(sendBuff));
    return;

  //project does not currently exist, so we create it
  }else if (ENOENT == errno){
    char command[strlen(project_name) + 6];
    strcpy(command,"mkdir ");
    strcat(command, project_name);
    system(command);

  //opendir() failed
  }else{
    printf("\nERROR: opendir() failed.\n");
    strcpy(sendBuff, "e");
    printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
    write(connfd, sendBuff, strlen(sendBuff));
    return;
  }

  //Create the .Manifest file
  int manifest_fd = open(".Manifest", O_WRONLY | O_TRUNC | O_CREAT, 0777);

    /*
    TODO: Communicate errors like these to clients in an efficient way
        -Make a method for only communicating errors
    */
  if(manifest_fd < 0){
    printf("\nError: Was unable to open/create the .Manifest file. Please try again.\n");
    exit(1);
  }

  int write_size = write(manifest_fd, "V 0\n", 4);
  if(write_size < 0){
    printf("\nError: Was unable to write to the .Manifest file. Please try again.\n");
    exit(1);
  }

  char filepath[10 + strlen(project_name)];
  strcpy(filepath, project_name);
  strcat(filepath, "/.Manifest");
  printf("\n%s\n", filepath);
  rename(".Manifest", filepath);
  /*
  TODO: Make the manifest file header more unique to each project
  */

  close(manifest_fd);
  //send manifest to client
  strcpy(sendBuff, "V 0\n");
  printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
  write(connfd, sendBuff, strlen(sendBuff));
  //cleanup
  closedir(dir);


}

void destroy(char* command, int connfd){
  //Parse project name from the message received from the client
  char project_name[strlen(command) - 1];
  memset(project_name, 0, sizeof(project_name));
  int i;
  for(i = 2; i < strlen(command); i++){
      if(command[i]){
        project_name[i - 2] = command[i];
      }

  }

  //Set up buffer for responding to client
  char sendBuff[1024];
  memset(sendBuff, '\0', sizeof(sendBuff));

  //Attempt to open directory with the name specified by the client
  DIR* dir = opendir(project_name);

  //Project does not exist so we can't destroy it, inform client
  if(ENOENT == errno){
    strcpy(sendBuff, "x");
    printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
    write(connfd, sendBuff, strlen(sendBuff));
    return;

  //project does exist so we destroy it
  }else if (dir){
    closedir(dir);
    char command[strlen(project_name) + 9];
    strcpy(command,"rm -rf ");
    strcat(command, project_name);
    system(command);

  //opendir() failed
  }else{
    printf("\nERROR: opendir() failed.\n");
    strcpy(sendBuff, "e");
    printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
    write(connfd, sendBuff, strlen(sendBuff));
    return;
  }


  //send success message to client
  strcpy(sendBuff, "Destroying the project was a success!");
  printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
  write(connfd, sendBuff, strlen(sendBuff));


}

/*Check out
--------------------------------------------------
If the server is sent the checkout command, it will send the
specified project's .Manifest and all files listed
in the .Manifest to the client.
*/
void checkout(char* command, int connfd){
  //Parse project name from the message received from the client
  char project_name[strlen(command) - 1];
  memset(project_name, 0, sizeof(project_name));
  int i;
  for(i = 2; i < strlen(command); i++){
      if(command[i]){
        project_name[i - 2] = command[i];
      }

  }

  //Attempt to open directory with the name specified by the client
  DIR* dir = opendir(project_name);

  //Report an error assuming the project does not exist
  if(!dir){
    closedir(dir);
    printf("Message: -%s- size: %d\n", "x", 1);
    write(connfd, "x", 1);
    return;
  }else if (dir){
    //do nothing and continue

  //opendir() failed
  }else{
    printf("\nERROR: opendir() failed.\n");
    printf("Message: -%s- size: %d\n", "e", 1);
    write(connfd, "e", 1);
    return;
  }

  //read .Manifest's contents
  char manifest_path[sizeof(project_name) + 11];
  strcpy(manifest_path, project_name);
  strcat(manifest_path, "/.Manifest");
  char* manifest_contents = readFile(manifest_path);

  //Will be storing all file paths in a char**
  char** file_paths = malloc(100 * sizeof(char*));
  for(i = 0; i < 100; i++){
    file_paths[i] = malloc(124 * sizeof(char));
  }
  int file_paths_idx = 0;

  //tokenize the manifest file by every line
  char* token;
  char* saveptr = manifest_contents;

  //loop through the tokens
  while(token = strtok_r(saveptr, "\n", &saveptr)){
    //store each file path in the manifest
    if(token[0] == '.'){
      int idx = 0;
      char file_path[strlen(token)];
      while(token[idx] != '~'){
        file_path[idx] = token[idx];
        idx++;
      }
      file_path[idx] = '\0';
      strcpy(file_paths[file_paths_idx], file_path);
      file_paths_idx++;
    }

  }

  //Adding .Manifest to the list of file paths
  strcpy(file_paths[file_paths_idx], project_name);
  strcat(file_paths[file_paths_idx], "/.Manifest");
  file_paths_idx++;

  //Getting a copy of the .Manifest to send to the client
  char* manifest = readFile(manifest_path);

  //Looping through all file paths, getting their sizes
  int file_sizes[file_paths_idx];

  for(i = 0; i < file_paths_idx - 1; i++){
    struct stat st;
    stat(file_paths[i], &st);
    file_sizes[i] = st.st_size;
  }
  file_sizes[file_paths_idx - 1] = strlen(manifest);

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
    sprintf(numStorage, "%d", file_sizes[i]);
    bytes_needed += strlen(numStorage);
    //the bytes it will take to write the size of the file's name
    sprintf(numStorage, "%d", strlen(file_paths[i]));
    bytes_needed += strlen(numStorage);
    bytes_needed += 4;
  }


  //Set up buffer for responding to client
  char sendBuff[bytes_needed];
  memset(sendBuff, '\0', sizeof(sendBuff));

  //Copy the number of files to sendBuff
  sprintf(numStorage, "%d", file_paths_idx);
  strcpy(sendBuff, numStorage);


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
    char* contents = readFile(file_paths[i]);
    strcat(sendBuff, contents);


  }
  strcat(sendBuff, "\0");

  printf("\n\nMessage: -%s- size: %d\n", sendBuff, strlen(sendBuff));
  write(connfd, sendBuff, strlen(sendBuff));

  closedir(dir);
  free(manifest_contents);
  for(i = 0; i < 100; i++){
    free(file_paths[i]);
  }
  free(file_paths);
  free(manifest);
}

/*Commit
--------------------------------
If the server is sent the commit command, it will send the most
updated version of the specified project's .Manifest to the client.
*/
void commit(char* command, int connfd){
  //Parse project name from the message received from the client
  char project_name[strlen(command) - 1];
  memset(project_name, 0, sizeof(project_name));
  int i;
  for(i = 2; i < strlen(command); i++){
      if(command[i]){
        project_name[i - 2] = command[i];
      }

  }

  //Set up buffer for responding to client
  char sendBuff[1024];
  memset(sendBuff, '\0', sizeof(sendBuff));

  //Attempt to open directory with the name specified by the client
  DIR* dir = opendir(project_name);

  //Check if it exists, report an error if not
  if(!dir){
    closedir(dir);
    strcpy(sendBuff, "e");
    printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
    write(connfd, sendBuff, strlen(sendBuff));
    return;
  }

  closedir(dir);

  //Open the needed Manifest file
  char manifestPath[sizeof(project_name) + 11];
  strcpy(manifestPath,project_name);
  strcat(manifestPath,"/.Manifest");

  int manifest_fd = open(manifestPath, O_RDONLY);

    /*
    TODO: Communicate errors like these to clients in an efficient way
        -Make a method for only communicating errors
    */

   //check if manifest exists, report an error if not
  if(manifest_fd < 0){
    close(manifest_fd);
    strcpy(sendBuff, "x");
    printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
    write(connfd, sendBuff, strlen(sendBuff));
    return;
  }

  close(manifest_fd);
  char* fileContents=readFile(manifestPath);

  printf("Message: -%s- size: %d", fileContents, strlen(fileContents));
  write(connfd, fileContents, strlen(fileContents));


}

/*
Current Version
----------------------------------------
If the server is sent the currentVersion command, it will recieve the name
of an exist project, and then give the currentVersion of the project
along with it each file and each files currentVersion.
*/
void currentVersion(char* command, int connfd){
  //Parse project name from the message received from the client
  char project_name[strlen(command) - 1];
  memset(project_name, 0, sizeof(project_name));
  int i;
  for(i = 2; i < strlen(command); i++){
      if(command[i]){
        project_name[i - 2] = command[i];
      }

  }

  //Set up buffer for responding to client
  char sendBuff[1024];
  memset(sendBuff, '\0', sizeof(sendBuff));

  //Attempt to open directory with the name specified by the client
  DIR* dir = opendir(project_name);

  //Report an error assuming the project does not exist
  if(ENOENT == errno){
    closedir(dir);
    strcpy(sendBuff, "x");
    printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
    write(connfd, sendBuff, strlen(sendBuff));
    return;

  //project does exist, so continue
  }else if (dir){
    //do nothing and continue

  //opendir() failed
  }else{
    printf("\nERROR: opendir() failed.\n");
    strcpy(sendBuff, "e");
    printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
    write(connfd, sendBuff, strlen(sendBuff));
    return;
  }

  //Open the needed Manifest file
  char* manifestPath = (char*)malloc(sizeof(project_name)+11);
  strcpy(manifestPath,project_name);
  strcat(manifestPath,"/.Manifest");

  int manifest_fd = open(manifestPath, O_RDONLY);

    /*
    TODO: Communicate errors like these to clients in an efficient way
        -Make a method for only communicating errors
    */
  if(manifest_fd < 0){
    printf("\nError: Was unable to open/create the .Manifest file. Please try again.\n");
    exit(1);
  }
  char* fileContents=readFile(manifestPath);
  int j=0;
  //read current project version
  char* currentChar=(char*)malloc(sizeof(char)*2);
  memset(currentChar,'\0',sizeof(currentChar));
  currentChar[0]=fileContents[j];
  strcpy(sendBuff,currentChar);
  j++;
  currentChar[0]=fileContents[j];
  strcat(sendBuff,currentChar);
  j++;
  while (fileContents[j]!='\n'){
      currentChar[0]=fileContents[j];
      strcat(sendBuff,currentChar);
      j++;
  }
  strcat(sendBuff,"\n");
  j++;

  //now we read all filenames and their version numbers. each iteration is one line
  // of .Manifest

  while(fileContents[j]!='\0'){
      //read and store in sendBuff until first ~
        while(fileContents[j]!='~'){
            currentChar[0]=fileContents[j];
            strcat(sendBuff,currentChar);
            j++;
        }
        j++;
        strcat(sendBuff,"~");

      //read and store in sendBuff until second ~
      while(fileContents[j]!='~'){
            currentChar[0]=fileContents[j];
            strcat(sendBuff,currentChar);
            j++;
        }


      //read and DON'T store until end of line except \n
      while(fileContents[j]!='\n'){
            j++;
        }
        strcat(sendBuff,"\n");
        j++;

  }


  printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
  write(connfd, sendBuff, strlen(sendBuff));

  //cleanup
  closedir(dir);
  close(manifest_fd);
  free(manifestPath);
  free(currentChar);
  free(fileContents);


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
  char * result=(char*)malloc(33*sizeof(char));
  memset(result,'\0',33);
  for(i = 0; i < MD5_DIGEST_LENGTH; i++){
     sprintf(result+2*i, "%02x", digest[i]);
  }

  return result;
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

  int bytes_read = read(file_fd, file_contents, size);
  file_contents[bytes_read] = '\0';
  close(file_fd);
  return file_contents;

}

/*Helper method for receiving a commit file
------------------------------
This will assist when the client runs the commit command and
it works successfully thus they are sending a .Commit file to the
server. The server will then store it in the commits folder of
the project.
*/
void recv_commit(char* command, int connfd){
  //Parse message from client
  char message[strlen(command) - 1];
  memset(message, 0, sizeof(message));
  int i;
  for(i = 2; i < strlen(command); i++){
      if(command[i]){
        message[i - 2] = command[i];
      }

  }

  //Tokenize to identify project name and .Commit contents
  char* token;
  char* saveptr = message;
  token = strtok_r(saveptr, " ", &saveptr);
  char* projectName = token;
  token = strtok_r(saveptr, " ", &saveptr);
  char commitFile[strlen(token) + 2];
  memset(commitFile, 0, strlen(token) + 2);
  strcpy(commitFile, token);


  //Add commit contents to a new file in the commits folder within the project, or create commits folder if it was not already created
  char commits_path[strlen(projectName) + 10];
  memset(commits_path, 0, strlen(projectName) + 8);
  strcpy(commits_path, "./");
  strcat(commits_path, projectName);
  strcat(commits_path, "/Commits");
  DIR* dir = opendir(commits_path);

  //If Commits folder does not exist in the project, make it and create the first commit file
  if(dir == NULL){
    int test = mkdir(commits_path, 0777);
    if(test < 0){
      char* msg = "\nError: Server was unable to create a commits folder for this project. Please try again.";
      write(connfd, msg, strlen(msg));
      return;
    }
    char commitFile_path[strlen(projectName) + 11];
    strcpy(commitFile_path, commits_path);
    strcat(commitFile_path, "/.1");
    int commitfd = open(commitFile_path, O_WRONLY | O_CREAT, 0777);
    if(commitfd < 0){
      char* msg = "\nError: Server was unable to create a commit file for this commit. Please try again.";
      write(connfd, msg, strlen(msg));
      //Should definitely delete the directory here (maybe?)
      return;
    }
    int bytes_written = write(commitfd, commitFile , strlen(commitFile));
    if(bytes_written < 0){
      char* msg = "\nError: Server was unable to create a commit file for this commit. Please try again.";
      write(connfd, msg, strlen(msg));
      //Should definitely delete the file here
      return;
    }
    char* msg = "\nStatus: Successful commit";
    write(connfd, msg, strlen(msg));
    close(commitfd);
    closedir(dir);

    //Commits folder exists, must create a new commit file with a unique name
  }else{

    struct dirent* ent;
    int newCommitName = 0;
    while((ent = readdir(dir)) != NULL){
      char* fname = ent->d_name;
      if(strcmp(fname, ".") == 0 || strcmp(fname, "..") == 0){
        continue;
      }
      char* tkn;
      char* svptr = fname;
      tkn = strtok_r(svptr, ".", &svptr);
      newCommitName = atoi(tkn);
    }

    newCommitName++;
    char nameBuffer[5];
    sprintf(nameBuffer, "%d\0", newCommitName);
    char newCommitFullName[strlen(nameBuffer) + 1];
    strcpy(newCommitFullName, ".");
    strcat(newCommitFullName, nameBuffer);


    char commitFile_path[strlen(projectName) + 8 + strlen(newCommitFullName)];
    strcpy(commitFile_path, projectName);
    strcat(commitFile_path, "/Commits/");
    strcat(commitFile_path, newCommitFullName);

    int commitfd = open(commitFile_path, O_WRONLY | O_CREAT, 0777);
    if(commitfd < 0){
      char* msg = "\nError: Server was unable to create a commit file for this commit. Please try again.";
      write(connfd, msg, strlen(msg));
      //Should definitely delete the directory here (maybe?)
      return;
    }
    int bytes_written = write(commitfd, commitFile , strlen(commitFile));
    if(bytes_written < 0){
      char* msg = "\nError: Server was unable to create a commit file for this commit. Please try again.";
      write(connfd, msg, strlen(msg));
      //Should definitely delete the file here
      return;
    }
    char* msg = "\nStatus: Successful commit";
    write(connfd, msg, strlen(msg));
    close(commitfd);
    closedir(dir);
  }
}

/*Helper method to handle the push command
---------------------------------------------
*/

void push(char* command, int connfd){
  //lock the repository
  pthread_mutex_lock(&lock);

  //Parse message from client
  char message[strlen(command) - 1];
  memset(message, 0, sizeof(message));
  int i;
  for(i = 2; i < strlen(command); i++){
      if(command[i]){
        message[i - 2] = command[i];
      }
      
  }

  //Isolate project name
  char* saveptr = message;
  char* projectName = strtok_r(saveptr, " ", &saveptr);

  char numberStorage[10];
  memset(numberStorage, '\0', 10);
  char* token = strtok_r(saveptr, "$", &saveptr);
  sprintf(numberStorage, "%s", token);

  int numFiles = atoi(numberStorage);

  token = strtok_r(saveptr, "&", &saveptr);
  token = strtok_r(saveptr, "&", &saveptr);
  token = strtok_r(saveptr, "&", &saveptr);
  char* commitContents = strtok_r(saveptr, "$", &saveptr);
  
  char* sendbuff;

  //Search for a commit file that matches the one that the client sent
  char commits_path[strlen(projectName) + 10];
  memset(commits_path, 0, strlen(projectName) + 10);
  strcpy(commits_path, "./");
  strcat(commits_path, projectName);
  strcat(commits_path, "/Commits");
  DIR* dir = opendir(commits_path);

  int does_commit_exist = 0;
  char* activeCommit = malloc(256 * sizeof(char));
  memset(activeCommit, 0, 256);


  //If commits directory does not exist, alert client they must create one first
  if(dir == NULL){
    closedir(dir);
    sendbuff = "\nError: no commits exist on the server currently. Run the commit command before you push.";
    write(connfd, sendbuff, strlen(sendbuff));
    pthread_mutex_unlock(&lock);
    return;
  }else{
    struct dirent* ent;
    int newCommitName = 0;
    //for each dirent in the directory
    while((ent = readdir(dir)) != NULL){
      char* fname = ent->d_name;
      //if its a folder, skip over it
      if(strcmp(fname, ".") == 0 || strcmp(fname, "..") == 0 || ent->d_type  == DT_DIR){
        continue;

      //if it is not a folder, we can assume that the dirent is a file. Thus, we can open it and compare its contents to the message from the client
      }else{
        char pathOfCurrentCommit[strlen(projectName) + strlen(ent->d_name) + 9];
        strcpy(pathOfCurrentCommit, projectName);
        strcat(pathOfCurrentCommit, "/Commits/");
        strcat(pathOfCurrentCommit, ent->d_name);
        char* contents = readFile(pathOfCurrentCommit);
        if(strcmp(contents, commitContents) == 0){
          strcpy(activeCommit, "./");
          strcat(activeCommit, pathOfCurrentCommit);
          does_commit_exist = 1;
          break;
        }
      }
    }
  }
  closedir(dir);
  //If the commit exists on the server, delete all others
  if(does_commit_exist == 1){
    DIR* dir2 = opendir(commits_path);
    struct dirent* ent;
    char filepath[256];
    
    while((ent = readdir(dir2)) != NULL){
      if(ent->d_type == DT_DIR){
        continue;
      }
      sprintf(filepath, "%s/%s", commits_path, ent->d_name);
      if(strcmp(filepath, activeCommit) != 0){
        remove(filepath);
      }
      
    }
    closedir(dir2);

  //If the commit does not exist on the server, report that to the client
  }else{
    sendbuff = "\nError: no commit file exists on the server that matches the one you pushed. Please run the commit command before pushing.";
    write(connfd, sendbuff, strlen(sendbuff));
    pthread_mutex_unlock(&lock);
    return;
  }



  

  //Get the current project's version number to name the archive
  char manifestPath[strlen(projectName) + 10];
  strcpy(manifestPath, projectName);
  strcat(manifestPath, "/.Manifest");
  char* manifest_contents = readFile(manifestPath);
  char* aptr = manifest_contents;
  char* version_num = strtok_r(aptr, "\n", &aptr);
  char* aptr2 = version_num;
  version_num = strtok_r(aptr2, " ", &aptr2);
  version_num = strtok_r(aptr2, " ", &aptr2);
  //Store the current project in an archive
  char historyPath[strlen(projectName) + 100];
  strcpy(historyPath, projectName);
  strcat(historyPath, "/History");
  strcat(historyPath, "/");
  DIR* historydir = opendir(historyPath);
  if(historydir == NULL){
    int result = mkdir(historyPath, 0777);
  }
  char cmd[100];
  sprintf(cmd, "cp -avr %s %s", projectName, version_num);
  system(cmd);
  sprintf(cmd, "mv %s %s", version_num, historyPath);
  system(cmd);
  
  int vnum = atoi(version_num);
  vnum++;

  //prepare the write to the new manifest
  char manifestWrite[1000];
  memset(manifestWrite, '\0', 1000);
  sprintf(manifestWrite, "V %d\n", vnum);
  
  token = saveptr;
  int idx = 0;
  while(1){
    if(token[idx] == '\0' || idx > strlen(token)){
      break;
    }
    char file_name_sz[5];
    int subidx = 0;
    while(token[idx] != '&'){
      file_name_sz[subidx] = token[idx];
      idx++;
      subidx++;
    }
    idx++;
    int fn_size = atoi(file_name_sz);
    char file_name[fn_size + 1];
    memset(file_name, '\0', fn_size + 1);

    for(i = 0; i < fn_size; i++){
      file_name[i] = token[idx];
      idx++;
    }
    
    char file_sz[5];
    subidx = 0;
    idx++;
    while(token[idx] != '&'){
      file_sz[subidx] = token[idx];
      idx++;
      subidx++;
    }
    idx++;
    int file_size = atoi(file_sz);
    char file[file_size + 1];
    memset(file, '\0', file_size + 1);
    for(i = 0; i < file_size; i++){
      file[i] = token[idx];
      idx++;
    }
    idx++;


    if(file_name[0] == 'A'){
      char fn[strlen(file_name)+1];
      memset(fn, '\0', strlen(file_name) + 1);
      for(i = 1; i < strlen(file_name); i++){
        fn[i-1] = file_name[i];
      }
      fn[i] = '\0';

      int newFileFd = open(fn, O_WRONLY | O_CREAT, 0777);
      int b = write(newFileFd, file, strlen(file));
      close(newFileFd);

      strcat(manifestWrite, fn);
      strcat(manifestWrite, "~V:0~H:");
      strcat(manifestWrite, hashIt(file));
      strcat(manifestWrite, "\n");
    }else if(file_name[0] == 'D'){
      char fn[strlen(file_name)+1];
      memset(fn, '\0', strlen(file_name) + 1);
      for(i = 1; i < strlen(file_name); i++){
        fn[i-1] = file_name[i];
      }
      fn[i] = '\0';
      char cmdd[strlen(fn) + 8];
      strcpy(cmdd, "rm -rf ");
      strcat(cmdd, fn);
      system(cmdd);

    }else{
      char fn[strlen(file_name)+1];
      memset(fn, '\0', strlen(file_name) + 1);
      for(i = 1; i < strlen(file_name); i++){
        fn[i-1] = file_name[i];
      }
      fn[i] = '\0';

      int newFileFd = open(fn, O_WRONLY | O_CREAT, 0777);
      int b = write(newFileFd, file, strlen(file));
      close(newFileFd);

      char* idk = readFile(manifestPath);
      char* apointer = strstr(idk, fn);
      char* tokenizing = strtok_r(apointer, "~", &apointer);
      tokenizing = strtok_r(apointer, "~", &apointer);
      apointer = tokenizing;
      tokenizing = strtok_r(apointer, ":", &apointer);
      tokenizing = strtok_r(apointer, ":", &apointer);
      
      int newNum = atoi(tokenizing);
      newNum++;
      char buff[5];
      sprintf(buff, "%d", newNum);

      strcat(manifestWrite, fn);
      strcat(manifestWrite, "~V:");
      strcat(manifestWrite, buff);
      strcat(manifestWrite, "~H:");
      strcat(manifestWrite, hashIt(file));
      strcat(manifestWrite, "\n");

    }
    


  }
  

  int manfd = open(manifestPath, O_WRONLY | O_TRUNC);
  write(manfd, manifestWrite, strlen(manifestWrite));
  
  

  
  //Unlock the repository*/
  free(manifest_contents);
  closedir(historydir);
  sendbuff = "\nSuccessful push";
  write(connfd, sendbuff, strlen(sendbuff));
  pthread_mutex_unlock(&lock);
}

    /*
    TODO: Make a function for communicating error messages to clients
    */


/*Helper method to handle the update command
----------------------------------------------
If the server recieves an update request, it will send over the Manifest of the
specified project.
*/
void update(char* command, int connfd){
  //Parse project name from the message received from the client
  char project_name[strlen(command) - 1];
  memset(project_name, 0, sizeof(project_name));
  int i;
  for(i = 2; i < strlen(command); i++){
      if(command[i]){
        project_name[i - 2] = command[i];
      }
      
  }

  //Set up buffer for responding to client
  char sendBuff[1024];
  memset(sendBuff, '\0', sizeof(sendBuff));

  //Attempt to open directory with the name specified by the client
  DIR* dir = opendir(project_name);

  //Report an error assuming the project does not exist
  if(ENOENT == errno){
    closedir(dir);
    strcpy(sendBuff, "x");
    printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
    write(connfd, sendBuff, strlen(sendBuff));
    return;

  //project does exist, so continue
  }else if (dir){
    //do nothing and continue

  //opendir() failed
  }else{
    printf("\nERROR: opendir() failed.\n");
    strcpy(sendBuff, "e");
    printf("Message: -%s- size: %d\n", sendBuff, strlen(sendBuff));
    write(connfd, sendBuff, strlen(sendBuff));
    return;
  }

  //Open the needed Manifest file
  char* manifestPath = (char*)malloc(sizeof(project_name)+11);
  strcpy(manifestPath,project_name);
  strcat(manifestPath,"/.Manifest");

  int manifest_fd = open(manifestPath, O_RDONLY);

    /*
    TODO: Communicate errors like these to clients in an efficient way
        -Make a method for only communicating errors
    */
  if(manifest_fd < 0){
    printf("\nError: Was unable to open/create the .Manifest file. Please try again.\n");
    exit(1);
  }
  char* fileContents=readFile(manifestPath);
  printf("Message: -%s- size: %d\n", fileContents, strlen(fileContents));
  write(connfd, fileContents, strlen(fileContents));
  return;
}

    /*
    TODO: Make a function for communicating error messages to clients
    */

      

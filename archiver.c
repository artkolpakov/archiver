/*
 programmed by Artem Kolpakov
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

/**
 * Like mkdir, but creates parent paths as well
 *
 * @return 0, or -1 on error, with errno set
 */
int mkpath(const char *pathname, mode_t mode){
  char *tmp = malloc(strlen(pathname) + 1);
  strcpy(tmp, pathname);
  for (char *p = tmp; *p != '\0'; ++p){
    if (*p == '/'){
      *p = '\0';
      struct stat st;
      if (stat(tmp, &st))
      {
        if (mkdir(tmp, mode))
        {
          free(tmp);
          return -1;
        }
      }
      else if (!S_ISDIR(st.st_mode)){
        free(tmp);
        return -1;
      }
      *p = '/';
    }
  }
  free(tmp);
  return 0;
}

/** 
 * Packs a single file or directory recursively
 *
 * @param fn The filename to pack
 * @param outfp The file to write encoded output to
 */
void pack(char * const fn, FILE *outfp){  //get's the length of the file name, the file contents and size
  // printf("Grabbed: %s\n", fn);
  struct stat st;                         
  stat(fn, &st);                          //get file status and info

    if (S_ISDIR(st.st_mode)){             //check if the current struct is a directory or file
      // fprintf(stderr, "Recursing `%s'\n", fn);
      chdir(fn);                          //change the directory to the current directory

      DIR* currDir = opendir(".");        //open current directory
      struct dirent *aDir;
      printf("Dir name: %s\n", fn);       //printing directory name

      strcat(fn, "/");                    //adding '/' to the end of directory name
      int dirNameLength = strlen(fn);     //getting length of directory name
      fprintf(outfp, "%d:%s", dirNameLength, fn);       //printing the length

      while((aDir = readdir(currDir)) != NULL){         //go through all the entries, grabbing random one till it's null
        // string compare the name of directory to "." or ".."
        if(strcmp(".", aDir->d_name) == 0 || strcmp("..", aDir->d_name) == 0){
          continue;     //if . or .. skip it
        }
        // printf("Recursing: %s\n", cwd);
        pack(aDir->d_name, outfp);        //grab all the potential leafs
      }
      fprintf(outfp, "0:");               //Saying we're done with that directory by printing 0:
      chdir("..");                        //cd back to where we started from
      fprintf(stderr, "Going back\n");
      closedir(currDir);
      return;
    }
    else if (S_ISREG(st.st_mode)){      //check if the current struct is a regular file
      FILE* file = fopen(fn, "r");      //if file opening fails 
      if(file == NULL){
        // fprintf(stderr, "Error opening a %s file!\n", fn);    //if file opening fails print an error
        err(errno, "fopen()");
        fclose(file);
        return;
      }
      int fileNameLength = strlen(fn);           //get filename length

      fprintf(stderr, "Packing `%s'\n", fn);
      fprintf(stderr, "Size of %s: %zu\n", fn, st.st_size);
      fprintf(outfp, "%d:%s%zu:", fileNameLength, fn, st.st_size);    //write to the file filename, length

      for (int i = 0; i < st.st_size; i++){         //reading contents
        char in[1] = {'\n'};                        //reading and writing 1 at a time
        fread(in, 1, 1, file);  
        fwrite(in, 1, sizeof(in), outfp);
      }
      fclose(file);                                  //close file
    }
    else{
      fprintf(stderr, "Skipping non-regular file `%s'.\n", fn); //the program only archives regular text files
    }
}

/**
 * Unpacks an entire archive
 *
 * @param fp The archive to unpack
//  */
int unpack(FILE *fp){
  //change and open (both)
  off_t size = 0;
  while(1){
      if (fscanf(fp, "%jd:", &size) <= 0){      //if EOF exit, reading a filename/dir length
        fprintf(stderr, "Done unpacking\n");
        break;                                  //break if we reach an end
      }
      if(size == 0){      //if there are no more file (whenever we hit a filename length of zero) go back to parent dir
        fprintf(stderr, "Going back, up a level in the directory structure\n");
        chdir("..");      //we cd back to where you started from (go up a level in the directory structure)
        continue;
      }
      char fn[size];                  //creating a buffer to store the filename
      memset(fn, '\0', size+1);
      fread(fn, 1, size, fp);         //reading the filename

      fprintf(stderr, "Read dir/filename: ");       
      // printf("size of read dir/file: %d\n", size);
      fwrite(fn, 1, size, stderr);      //printing a size
      fprintf(stderr, "\n");

      if (fn[strlen(fn)-1] == '/'){
        // fprintf(stderr, "Unpacking a dir %s\n", fn);
        if (mkpath(fn, 0700)) err(errno, "mkpath()");         //create a new dir
        fprintf(stderr, "Recursing into `%s'\n", fn);
        chdir(fn);
        unpack(fp);                     //recursively visit other nodes
        }
      else{
        // fprintf(stderr, "Unpacking file %s\n", fn);
        off_t sizeOfContents = 0;
        fscanf(fp, "%jd:", &sizeOfContents);      //reading size of file contents
        fprintf(stderr, "Read size of file: %jd\n",sizeOfContents);
        FILE *outfp = fopen(fn, "w");
        if(outfp == NULL){
          // fprintf(stderr, "Error opening a %s file", fn);
          err(errno, "fopen()");
          fclose(outfp);
          return 1;
        }
        for (int i = 0; i < sizeOfContents; i++){   //reading data 1 at a time and printing it to a file
          char in[1] = {'\n'};
          fread(in, 1, 1, fp);
          fwrite(in, 1, sizeof(in), outfp);
        }
        fclose(outfp);
      }
    }
    return 0;
}

int main(int argc, char *argv[]){
  if (argc < 2) {                                               //Program prints usage message if invalid number of arguments is provided
    fprintf(stderr, "Usage: %s FILE... OUTFILE\n"
                    "       %s INFILE\n", argv[0], argv[0]);
    exit(1);
  }
  char *fn = argv[argc-1];
  if (argc > 2) { /* Packing files */
    FILE *fp = fopen(fn, "w");
    if (fp == NULL) {                                             //Program prints error if file open fails
      // fprintf(stderr, "Opening file/dir to pack failed: No such file or directory\n");
      err(errno, "fopen() error");
      exit(1);
    }

    int argcCoppy = argc;
    char arr[argc-2][256];
    for(int i = 1; i < argc -1; i++){
      strcpy(arr[i], argv[i]);
      // printf("Coppied %s\n", arr[i]);
    }
    //for some reason argc and argv reset to 0 and '' after pack is called, so I fixed it by making a copy
    for (int argind = 1; argind < argcCoppy - 1; ++argind){
      pack(arr[argind], fp);                                     //pack all of the given arguments, write to newly created .arch file
    }
    fclose(fp);                                                   //close file
  }

  else { /* Unpacking an archive file */
    FILE *fp = fopen(fn, "r");
    if (fp == NULL) {   
      // fprintf(stderr, "Opening archive file failed: No such file or directory\n");
      err(errno, "fopen() error");
      exit(1);                                // exit
    }
    unpack(fp);                               //unpack given .arch file
    fclose(fp);                               //close .arch file
  }
  return EXIT_SUCCESS;
}

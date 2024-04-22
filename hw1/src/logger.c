#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

static FILE* (*o_fopen)(const char *, const char *) = NULL;
static size_t (*o_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
static size_t (*o_fwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
static int (*o_connect)(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = NULL;
static int (*o_getaddrinfo)(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) = NULL;
static int (*o_system)(const char *command) = NULL;

typedef struct blacknode{
  // int
  char *path;
  struct blacknode *next;
}blacknode;

void list_add(blacknode **head, char *path){
  blacknode *new_node = (blacknode *)malloc(sizeof(blacknode));
  new_node->path = strdup(path);
  new_node->next = *head;
  *head = new_node;
  // printf("added %s\n", path);
}

blacknode *list_search(blacknode *head, const char *path, int opt){
  blacknode *current = head;
  while(current != NULL){
    if(opt == 1){
      if(strstr(path, current->path)){
        return current;
      }
    }
    else{
      if(!strncmp(current->path, path, strlen(current->path))){
        return current;
      }
    }
    current = current->next;
  }
  return NULL;
}

void list_show(blacknode *head){
  if(!head){
    printf("empty\n");
    return;
  }
  blacknode *current = head;
  while(current != NULL){
    printf("%s\n", current->path);
    current = current->next;
  }
}

blacknode *open_blacklist = NULL;
blacknode *read_blacklist = NULL;
blacknode *write_blacklist = NULL;
blacknode *connect_blacklist = NULL;
blacknode *getaddrinfo_blacklist = NULL;

void blacklist_init(){
  FILE *fp = o_fopen(getenv("CONFIG"), "r");
  if(fp == NULL){
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  while((read = getline(&line, &len, fp)) != -1){
    if(!strcmp(line, "BEGIN open-blacklist\n")){
      getline(&line, &len, fp);
      while(strcmp(line, "END open-blacklist\n")){
        char *path = strdup(line);
        if(path[strlen(path)-2] == '*'){
          path[strlen(path)-2] = '\0';
        }
        else path[strlen(path)-1] = '\0';

        list_add(&open_blacklist, path);
        free(path);
        getline(&line, &len, fp);
      }
    }
    else if(!strcmp(line, "BEGIN read-blacklist\n")){
      getline(&line, &len, fp);
      while(strcmp(line, "END read-blacklist\n")){
        char *path = strdup(line);
        if(path[strlen(path)-2] == '*'){
          path[strlen(path)-2] = '\0';
        }
        else path[strlen(path)-1] = '\0';
        // printf("%s\n", path);
        list_add(&read_blacklist, path);
        free(path);
        getline(&line, &len, fp);
      }
    }
    else if(!strcmp(line, "BEGIN write-blacklist\n")){
      getline(&line, &len, fp);
      while(strcmp(line, "END write-blacklist\n")){
        char *path = strdup(line);
        if(path[strlen(path)-2] == '*'){
          path[strlen(path)-2] = '\0';
        }
        else path[strlen(path)-1] = '\0';
        list_add(&write_blacklist, path);
        free(path);
        getline(&line, &len, fp);
      }
    }
    else if(!strcmp(line, "BEGIN connect-blacklist\n")){
      getline(&line, &len, fp);
      while(strcmp(line, "END connect-blacklist\n")){
        char *path = strdup(line);
        path[strlen(path)-1] = '\0';
        list_add(&connect_blacklist, path);
        free(path);
        getline(&line, &len, fp);
      }
    }
    else if(!strcmp(line, "BEGIN getaddrinfo-blacklist\n")){
      getline(&line, &len, fp);
      while(strcmp(line, "END getaddrinfo-blacklist")){
        char *path = strdup(line);
        if(path[strlen(path)-2] == '*'){
          path[strlen(path)-2] = '\0';
        }
        else path[strlen(path)-1] = '\0';
        list_add(&getaddrinfo_blacklist, path);
        free(path);
        getline(&line, &len, fp);
      }
    }

  }
  free(line);
  fclose(fp);
}

void __attribute__((constructor)) init(void){
  o_fopen = dlsym(RTLD_NEXT, "fopen");
  o_fread = dlsym(RTLD_NEXT, "fread");
  o_fwrite = dlsym(RTLD_NEXT, "fwrite");
  o_connect = dlsym(RTLD_NEXT, "connect");
  o_system = dlsym(RTLD_NEXT, "system");
  o_getaddrinfo = dlsym(RTLD_NEXT, "getaddrinfo");
  blacklist_init();
  // list_show(open_blacklist);
  // list_show(read_blacklist);
  // list_show(write_blacklist);
  // list_show(connect_blacklist);
  // list_show(getaddrinfo_blacklist);
}

void process_escape_string(char *str1, char *str2){
  char *src = str1;
  char *dst = str2;
  while(*src){
    switch(*src){
      case '\n':
        *dst++ = '\\';
        *dst++ = 'n';
        break;
      case '\t':
        *dst++ = '\\';
        *dst++ = 't';
        break;
      case '\\':
        *dst++ = '\\';
        *dst++ = '\\';
        break;
      case '\"':
        *dst++ = '\\';
        *dst++ = '"';
        break;
      case '\'':
        *dst++ = '\\';
        *dst++ = '\'';
        break;
      case '\r':
        *dst++ = '\\';
        *dst++ = 'r';
        break;
      case '\a':
        *dst++ = '\\';
        *dst++ = 'a';
        break;
      case '\b':
        *dst++ = '\\';
        *dst++ = 'b';
        break;
      default:
        *dst++ = *src;
        break;
    }
    src++;
  }
  *dst = '\0';
}

char* myrealpath(const char *path, char* resolved_path){
  if(path[0] == '/'){
    strcpy(resolved_path, path);
    return resolved_path;
  }
  else{
    return realpath(path,resolved_path);
  }
}

char* getpath(FILE *stream){
  int fd = fileno(stream);
  char path[256];
  memset(path, 0, 256);

  char fd_link[256];
  sprintf(fd_link, "/proc/%d/fd/%d", getpid(), fd);
  ssize_t len = readlink(fd_link, path, 256);
  if(len == -1){
    perror("readlink");
    exit(EXIT_FAILURE);
  }
  return strdup(path);
}

FILE* fopen(const char *path, const char *mode){
  FILE *fp = NULL;
  char p[256];
  memset(p,0,256);
  struct stat st;
  int x;
  x = lstat(path, &st);
  if(S_ISLNK(st.st_mode)) readlink(path, p, 256);
  else myrealpath(path,p);
  if(list_search(open_blacklist, p, 0) != NULL){
    errno = EACCES;
  }
  else fp = o_fopen(p, mode);

  if(!fp) fprintf(stderr, "[logger] fopen(\"%s\", \"%s\") = 0x0\n", path, mode);
  else fprintf(stderr, "[logger] fopen(\"%s\", \"%s\") = %p\n", path, mode, fp);
  return fp;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream){
  char *path = getpath(stream);
  char *filename = strtok(basename(path), ".");
  memset(ptr, 0, size*nmemb+1);
  char logfile[256];
  sprintf(logfile, "%d-%s-read.log", getpid(), filename);
  free(path);

  size_t ret = o_fread(ptr, size, nmemb, stream);
  if(list_search(read_blacklist, (char*)ptr,1) != NULL){
    errno = EACCES;
    fseek(stream, -(size * nmemb), SEEK_CUR);
    memset(ptr, 0, size * nmemb);
    ret = 0;
  }else{
    FILE *log_fp = o_fopen(logfile, "a+");
    fprintf(log_fp, "%s\n", (char*)ptr);
    fclose(log_fp);
  }

  fprintf(stderr, "[logger] fread(%p, %lu, %lu, %p) = %ld\n", ptr, size, nmemb, stream, ret);
  return ret;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream){
  char *path = getpath(stream);

  if(list_search(write_blacklist, path, 2) != NULL){
    errno = EACCES;
    free(path);
    return 0;
  }
  char *filename = strtok(basename(path), ".");
  free(path);
  char logfile[256];
  sprintf(logfile, "%d-%s-write.log", getpid(), filename);
  char str[256];
  memset(str, 0, 256);
  process_escape_string((char*)ptr, str);
  FILE *log_fp = o_fopen(logfile, "a+");
  o_fwrite(str, size, strlen(str), log_fp);
  fprintf(log_fp, "\n");
  fclose(log_fp);
  
  size_t ret = o_fwrite(str, size, nmemb, stream);
  fprintf(stderr, "[logger] fwrite(\"%s\", %lu, %lu, %p) = %ld\n", str, size, nmemb, stream, ret);
  return ret;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
  struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
  char server_ip[256];
  inet_ntop(AF_INET, &addr_in->sin_addr, server_ip, 256);

  int ret = 0;
  if(list_search(connect_blacklist, server_ip,3) != NULL){
    errno = ECONNREFUSED;
    ret = -1;
  }
  else ret = o_connect(sockfd, addr, addrlen);

  fprintf(stderr, "[logger] connect(%d, \"%s\", %d) = %d\n", sockfd, server_ip, addrlen, ret);
  return ret;
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res){
  int ret = 0;
  if(list_search(getaddrinfo_blacklist, node, 4) != NULL){
    ret = EAI_NONAME;
  }
  else ret = o_getaddrinfo(node, service, hints, res);
  fprintf(stderr, "[logger] getaddrinfo(\"%s\", %p, %p, %p) = %d\n", node, service, hints, res, ret);
  return ret;
}

int system(const char *command){
  int ret = o_system(command);
  fprintf(stderr, "[logger] system(\"%s\") = %d\n", command, ret);
  return ret;
}

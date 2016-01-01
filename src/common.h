#define bool int
#define true 1
#define false 0

#ifdef DEBUG
#define BUFFSIZE 4 
#else
#define BUFFSIZE 1024
#endif

#define STD_ERR 2
#define STD_OUT 1
extern void error(char* err, bool critical, int fd);
extern void stderror(int fd);
extern char *itoa(int n);
extern char* concat(char *s1, char *s2) ;

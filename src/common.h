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

#define CHECK(a, err) if(a == -1) { error(err, true, 2); }
#define CHECKP(a, err) if(a == NULL) { error(err, true, 2); }

extern void error(char * err, bool critical, int fd);
extern void stderror(int fd);
extern char *itoa(int n);
extern char *concat(char *s1, char *s2) ;
extern bool get_silent();
extern void set_silent(bool silent);

#define PRINT(msg, out, err) \
	if(write(out, msg, strlen(msg)) < 0) \
stderror(err);

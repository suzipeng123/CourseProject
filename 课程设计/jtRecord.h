#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#define KEYLEN sizeof(int)
#define FileNAME "jing.hash"
#define RECORDLEN 32

struct jtRecord
{
int key;
char other[RECORDLEN-sizeof(int)];
};

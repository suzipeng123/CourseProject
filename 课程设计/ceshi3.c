#include<stdio.h>
#include<stdlib.h>
#include"jtRecord.h"
#include"HashFile.h"

void showHashFile();

int main(int argc,char *argv[])
{
struct jtRecord rec[18] = {
{1,"sometimes"},{2,"peopele"},{3,"cry"},
{4,"not"},{5,"because"},{6,"they"},
{7,"are"},{8,"weak"},{9,"it"},{10,"is"},
{11,"because"},{12,"they"},{13,"have"},
{14,"been"},{15,"strong"},{16,"for"},
{17,"too"},{18,"long"}
};
int j=0;
for(j=0;j<18;j++)
{
printf("<%d,%d>\t",rec[j].key,hash(KEYLEN,&rec[j],18));
}
int fd = hashfile_creat(FileNAME,O_RDWR|O_CREAT,RECORDLEN,18);
int i=0;
printf("\nOpen and Save Record...\n");
fd = hashfile_open(FileNAME,O_RDWR,0);
for(i=0;i<18;i++)
{
hashfile_saverec(fd,KEYLEN,&rec[i]);
}
hashfile_close(fd);
showHashFile();
//Demo find Rec
printf("\nFind Record...");
fd = hashfile_open(FileNAME,O_RDWR,0);
int offset = hashfile_findrec(fd,KEYLEN,&rec[4]);
printf("\noffset is %d\n",offset);
hashfile_close(fd);
struct jtRecord jt;
struct CFTag tag;
fd = open(FileNAME,O_RDWR);
lseek(fd,offset,SEEK_SET);
read(fd,&tag,sizeof(struct CFTag));
printf("Tag is <%d,%d>\t",tag.collision,tag.free);
read(fd,&jt,sizeof(struct jtRecord));
printf("Record is {%d,%s}\n",jt.key,jt.other);
//Demo Delete Rec
printf("\nDelete Record...");
fd = hashfile_open(FileNAME,O_RDWR,0);
hashfile_delrec(fd,KEYLEN,&rec[2]);
hashfile_close(fd);
showHashFile();
//Demo Read
fd = hashfile_open(FileNAME,O_RDWR,0);
char buf[32];
memcpy(buf,&rec[1],KEYLEN);
hashfile_read(fd,KEYLEN,buf);
printf("\nRead Record is {%d,%s}\n",
((struct jtRecord *)buf)->key,((struct jtRecord *)buf)->other);
hashfile_close(fd);
//Demo Write
printf("\nWrite Record...");
fd = hashfile_open(FileNAME,O_RDWR,0);
hashfile_write(fd,KEYLEN,&rec[3]);
hashfile_close(fd);
showHashFile();
return 0;
}

void showHashFile()
{
int fd;
printf("\n");
fd = open(FileNAME,O_RDWR);
lseek(fd,sizeof(struct HashFileHeader),SEEK_SET);
struct jtRecord jt;
struct CFTag tag;
while(1)
{
if(read(fd,&tag,sizeof(struct CFTag))<=0)
break;
printf("Tag is <%d,%d>\t",tag.collision,tag.free);
if(read(fd,&jt,sizeof(struct jtRecord))<=0)
break;
printf("Record is {%d,%s}\n",jt.key,jt.other);
}
close(fd);
}

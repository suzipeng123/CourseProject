#include<stdio.h>
#include<stdlib.h>
#include"jtRecord.h"
#include"HashFile.h"

void showHashFile();

int main(int argc,char *argv[])
{
struct jtRecord rec[8] = {
{1,"zhao"},{2,"qian"},{3,"sun"},
{4,"li"},{5,"zhou"},{6,"wu"},{7,"zheng"},{8,"wang"}
};
int j=0;
for(j=0;j<8;j++)
{
printf("<%d,%d>\t",rec[j].key,hash(KEYLEN,&rec[j],8));
}
int fd = hashfile_creat(FileNAME,O_RDWR|O_CREAT,RECORDLEN,8);
int i=0;
printf("\nOpen and Save Record...\n");
fd = hashfile_open(FileNAME,O_RDWR,0);
for(i=0;i<8;i++)
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

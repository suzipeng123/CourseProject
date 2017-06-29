#include<stdio.h>
#include<stdlib.h>
#include"HashFile.h"
#include"jtRecord.h"

//创建函数
int hashfile_creat(const char *filename,mode_t mode,int reclen,int total_rec_num)
{
struct HashFileHeader hfh;
int fd;
int rtn;
char *buf;
int i=0;
hfh.sig = 1233333;   //文件标识号
hfh.reclen = reclen;
hfh.total_rec_num = total_rec_num;
hfh.current_rec_num = 0;
fd = creat(filename,mode);
if(fd != -1)
{
rtn = write(fd,&hfh,sizeof(struct HashFileHeader));   // 写入文件头信息
if(rtn != -1)
{
buf = (char*)malloc((reclen+sizeof(struct CFTag))*total_rec_num);    // 创建记录空间
memset(buf,0,(reclen+sizeof(struct CFTag))*total_rec_num);   
rtn = write(fd,buf,(reclen+sizeof(struct CFTag))*total_rec_num);
free(buf);
}
close(fd);
return rtn;
}
else{
close(fd);
return -1;
}
}

//打开文件函数
int hashfile_open(const char *filename,int flags,mode_t mode)
{
int fd = open(filename,flags,mode);
struct HashFileHeader hfh;
if(read(fd,&hfh,sizeof(struct HashFileHeader))!= -1)
{
lseek(fd,0,SEEK_SET);
if(hfh.sig == 1233333)
return fd;
else
return -1;
}
else return -1;
}

//关闭文件函数
int hashfile_close(int fd)
{
return close(fd);
}

//读取记录函数
int hashfile_read(int fd,int keylen,void *buf)
{
struct HashFileHeader hfh;
readHashFileHeader(fd,&hfh);
int offset = hashfile_findrec(fd,keylen,buf);
if(offset != -1)
{
lseek(fd,offset+sizeof(struct CFTag),SEEK_SET);
return read(fd,buf,hfh.reclen);
}
else
{
return -1;
}
}

//写入记录函数
int hashfile_write(int fd,int keylen,void *buf)
{
return hashfile_saverec(fd,keylen,buf);
}

int hashfile_delrec(int fd,int keylen,void *buf)
{
if(checkHashFileEmpty(fd))
{
printf("this file is empty !");
return -1;
}
int offset;
offset = hashfile_findrec(fd,keylen,buf);
if(offset != -1)
{
struct CFTag tag;
read(fd,&tag,sizeof(struct CFTag));
tag.free =0; //置空闲标志 
lseek(fd,offset,SEEK_SET);
write(fd,&tag,sizeof(struct CFTag));
struct HashFileHeader hfh;
readHashFileHeader(fd,&hfh);
int addr = hash(keylen,buf,hfh.total_rec_num);
offset = sizeof(struct HashFileHeader)+addr*(hfh.reclen+sizeof(struct CFTag));
if(lseek(fd,offset,SEEK_SET)==-1)
return -1;
read(fd,&tag,sizeof(struct CFTag));
tag.collision--; //冲突计数减1
lseek(fd,offset,SEEK_SET);
write(fd,&tag,sizeof(struct CFTag));
hfh.current_rec_num--; //当前记录数减1
lseek(fd,0,SEEK_SET);
write(fd,&hfh,sizeof(struct HashFileHeader));
}
else{
return -1;
}
}

//查询记录函数
int hashfile_findrec(int fd,int keylen,void *buf)
{
if(checkHashFileEmpty(fd))
{
printf("this file is empty !");
return -1;
}
struct HashFileHeader headMessage;
readHashFileHeader(fd, &headMessage);
int address = hash(keylen,buf,headMessage.total_rec_num);
int offset = sizeof(struct HashFileHeader) + address*(headMessage.reclen+sizeof(struct CFTag));
if(lseek(fd, offset, SEEK_SET) == -1)
 return -1;
struct CFTag tag;
read(fd, &tag, sizeof(struct CFTag));
int collision = tag.collision;
if(collision == 0){     //冲突为0时说明free也为0,record为空
 printf("\n1 this record is not exisit !");
 return -1;
}
int flag=0;
int count=0;
while(1){
 flag++;      //记录查询次数
 if(tag.free == 0){         //针对原记录被删除的情况
  offset += headMessage.reclen + sizeof(struct CFTag);
  if(offset >= lseek(fd, 0, SEEK_END))   //探寻到文件末尾
   offset = sizeof(struct HashFileHeader);
  if(lseek(fd, offset, SEEK_SET) == -1)
   return -1;
  read(fd, &tag, sizeof(struct CFTag));
 }else{
  char *record = (char*)malloc(headMessage.reclen*sizeof(char)); //
  read(fd, record, headMessage.reclen);
  //比较记录
  char *p1,*p2;
  p1 = (char *)buf;
  p2 = record;
  count=0;
  while((*p1 == *p2)&&(count<keylen)){ //逐位进行比较    
   p1++;
   p2++;
   count++;
 }
 if(count == keylen){
   free(record);
   record = NULL;
   return offset;
 }else{
 if(address == hash(keylen,record,headMessage.total_rec_num))  //其他都满足但内容不对应
 {
 collision--;
 if(collision == 0)
 {
 free(record);
 record = NULL;
 return -1;       //不存在 
  }
}
}
 free(record);
 record = NULL;
 offset += headMessage.reclen + sizeof(struct CFTag);
 if(offset >= lseek(fd, 0, SEEK_END))   //探寻到文件末尾
   offset = sizeof(struct HashFileHeader);
 if(lseek(fd, offset, SEEK_SET) == -1)
   return -1;
  read(fd, &tag, sizeof(struct CFTag));
}
if(flag == headMessage.total_rec_num){   //查询次数等于记录总数退出
 printf("\n2 this record is not exisit !");
 return -1;
 }
}
}

//保存记录函数
int hashfile_saverec(int fd,int keylen,void *buf)
{
if(checkHashFileFull(fd)){
  printf("warning: file is full !!!");
  return -1;
}
struct HashFileHeader headerMessage;
readHashFileHeader(fd, &headerMessage);
int address = hash(keylen, buf, headerMessage.total_rec_num);
int offset = sizeof(struct HashFileHeader) + address*(headerMessage.reclen + sizeof(struct CFTag));
if(lseek(fd,offset,SEEK_SET) == -1)  
  return -1;
struct CFTag tag;
read(fd, &tag, sizeof(struct CFTag));
tag.collision++; 
lseek(fd, sizeof(struct CFTag)*(-1), SEEK_CUR);
write(fd, &tag, sizeof(struct CFTag));
while(tag.free!=0){
 offset += headerMessage.reclen + sizeof(struct CFTag);
 if(offset >= lseek(fd, 0, SEEK_END))   //探寻到文件末尾
  offset = sizeof(struct HashFileHeader);
 if(lseek(fd, offset, SEEK_SET) == -1)
  return -1;
  read(fd, &tag, sizeof(struct CFTag)); //为下一次循环准备  
}
tag.free = -1; //文件置为占用
lseek(fd, sizeof(struct CFTag)*(-1), SEEK_CUR);
write(fd, &tag, sizeof(struct CFTag));
write(fd,buf,headerMessage.reclen);   // write record into hashfile
headerMessage.current_rec_num++;
lseek(fd, 0, SEEK_SET);
return write(fd, &headerMessage, sizeof(struct HashFileHeader));    //存入数据
}

//计算hash值函数
int hash(int keylen,void *buf,int total_rec_num)
{
int i=0;
char *p =(char*)buf;
int addr =0;
for(i=0;i<keylen;i++)
{
addr += (int)(*p);
p++;
}
return addr%(int)(total_rec_num*COLLISIONFACTOR);
}

//检查文件是否已满函数
int checkHashFileFull(int fd)
{
struct HashFileHeader headMessage;
readHashFileHeader(fd,&headMessage);
if(headMessage.current_rec_num<headMessage.total_rec_num)
return 0;
else
return 1;
}

int checkHashFileEmpty(int fd)
{
struct HashFileHeader headMessage;
readHashFileHeader(fd, &headMessage);
if(headMessage.reclen == 0)
return 1;
else
return 0;
}

//读取文件头信息函数
int readHashFileHeader(int fd,struct HashFileHeader *hfh)
{
lseek(fd,0,SEEK_SET);
return read(fd,hfh,sizeof(struct HashFileHeader));
}

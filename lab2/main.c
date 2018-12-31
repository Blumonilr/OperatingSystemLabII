#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#pragma pack (1)
struct BPB{
    short BPB_BytsPerSec;
    char BPB_SecPerClus;
    short BPB_RsvdSecCnt;
    char BPB_NumFATs;
    short BPB_RootEntCnt;
    short BPB_TotSec16;
    char BPB_Media;
    short BPB_FATSz16;
    short BPB_SecPerTrk;
    short BPB_NumHeads;
    int BPB_HideSec;
    int BPB_TotSec32;
};

#pragma pack (1)
struct Root{
    char DIR_Name[0xB];
    char DIR_Attr;
    char DIR_Rsvd[10];
    short DIR_WrtTime;
    short DIR_WrtDate;
    short DIR_FstClus;
    int DIR_FileSize;
};

#pragma pack (1)
struct File{
    char fileName[70];
    int isFile;
    unsigned int startClus;
    int isCount;
};

int fileNum = 0;
int BytsPerSec;
int RsvdSecCnt;
int NumFATs;
int RootEntCnt;
int FATSz16;
int BytsPerClus;
struct File files[20];

FILE* fat;

void my_print(char* c, int length, int color);
int isFile(char filename[]);
void loadBPB(FILE* file, struct BPB* bpb);
void loadFiles(FILE* file);
void readDirName(char dir[],char DIR_Name[]);
void readFileName(char filename[],char DIR_Name[]);
void readDataSec(FILE* file, char dir[], int DIR_FstClus);
void operate();
void addFile(char fileName[], int DIR_FstClus, int isFile);
void lsPrint(char dir[]);
void cutStr(char path[],char name[],char dir[]);
void simply(char name[],char path[]);
void catFile(FILE* file, int clusNum);
void countDir(char path[],int parent);
void printNum(int num);

int main(){
    fat = fopen("a.IMA", "rb");
    struct BPB bpb_ins;
    struct BPB* bpb=&bpb_ins;
    loadBPB(fat, bpb);
    loadFiles(fat);
    operate();
    return 0;
}

void loadBPB(FILE* file, struct BPB* bpb){
    fseek(file, 11, SEEK_SET);
    fread(bpb, 1, 25, file);
    BytsPerSec = bpb->BPB_BytsPerSec;
    RsvdSecCnt = bpb->BPB_RsvdSecCnt;
    NumFATs = bpb->BPB_NumFATs;
    RootEntCnt = bpb->BPB_RootEntCnt;
    FATSz16 = bpb->BPB_FATSz16;
    BytsPerClus = bpb->BPB_BytsPerSec*bpb->BPB_SecPerClus;
}

void loadFiles(FILE* file){
    struct Root root_ins;
    struct Root* root=&root_ins;
    int start = (RsvdSecCnt + NumFATs*FATSz16)*BytsPerSec;
    int index = 0;
    while (index <= RootEntCnt) {
        fseek(file, start + index * 32, 0);
        fread(root, 1, 32, file);

        if (!isFile(root->DIR_Name)) {
            index++;
            continue;
        }

        if (root->DIR_Attr == 0x10) {
            char dir[15];
            readDirName(dir,root->DIR_Name);
            addFile(dir, root->DIR_FstClus, 0);
            readDataSec(file, dir, root->DIR_FstClus);
        } else {
            char filename[15];
            readFileName(filename,root->DIR_Name);
            addFile(filename, root->DIR_FstClus, 1);
        }
        index++;
    }
}

void readDirName(char dir[],char DIR_Name[]){
    for (int i = 0; i < 11; i++){
        if (DIR_Name[i] == ' '){
            dir[i]='\0';
            break;
        }
        else
            dir[i] = DIR_Name[i]-'A'+'a';
    }
    return;
}

void readFileName(char filename[],char DIR_Name[]){
    int i=0;
    for (; i < 8; i++){
        if (DIR_Name[i] == ' '){
            break;
        }
        else
            filename[i] = DIR_Name[i]-'A'+'a';
    }
    filename[i] = '.';
    for (int j = 8; j < 11; j++){
        filename[++i] = DIR_Name[j]-'A'+'a';
    }
    filename[i+1]='\0';
    return;
}

void readDataSec(FILE* file, char dir[], int DIR_FstClus){
    int dataStart = (RsvdSecCnt + NumFATs * FATSz16 + ((RootEntCnt * 32) + (BytsPerSec - 1)) / BytsPerSec) * BytsPerSec;
    struct Root root_ins;
    struct Root* root=&root_ins;
    int index = 0;
    while (index < BytsPerClus){
        fseek(file, dataStart + BytsPerClus*(DIR_FstClus - 2) + index, 0);
        fread(root, 1, 32, file);

        if(!isFile(root-> DIR_Name)) {
            index=index+32;
            continue;
        }

        if (root->DIR_Attr == 0x10){
            char listFileName[15];
            readDirName(listFileName,root->DIR_Name);
            char dirName[70];
            strcpy(dirName,dir);
            strcat(dirName,"/");
            strcat(dirName,listFileName);
            addFile(dirName,root->DIR_FstClus,0);
            readDataSec(file, dirName,root->DIR_FstClus);
        }
        else{
            char fileName[15];
            readFileName(fileName,root->DIR_Name);
            char filePath[70];
            strcpy(filePath,dir);
            strcat(filePath,"/");
            strcat(filePath,fileName);
            addFile(filePath, root->DIR_FstClus, 1);
        }
        index=index+32;
    }

}

int isFile(char filename[]) {
    for(int i = 0; i < 11; i++) {
        if(!((filename[i] >= '0' && filename[i] <= '9') || (filename[i] >= 'a' && filename[i] <= 'z') || (filename[i] >= 'A' && filename[i] <= 'Z') || filename[i] == 0x20) ){
            return 0;
        }
    }
    return 1;
}

void addFile(char fileName[], int DIR_FstClus,int isFile){
    strcpy(files[fileNum].fileName,fileName);
    files[fileNum].startClus = DIR_FstClus;
    files[fileNum].isFile = isFile;
    files[fileNum].isCount=0;
    fileNum++;
}

void operate(){
    char input[]="Please input your operation: \n";
    char pathNotFound[]="Path not found!\n";
    char notAPath[]=" is not a directory!\n";
    char wrgOpe[]="Wrong operation!\n";
    char notAFile[]=" is not a file!\n";
    char opr[50];
    while (1){
        //printf("%s",input);
	my_print(input,strlen(input),0);
        scanf("%[^\n]",opr);
        getchar();
        if(strcmp(opr,"exit")==0){
            break;
        }else if(strcmp(opr,"ls")==0) {
            //printf("/:\n");
	    my_print("/:\n",4,0);
            for(int i=0;i<fileNum;i++) {
                if (strstr(files[i].fileName, "/")==NULL){
                    //printf("%s ", files[i].fileName);
		    if(files[i].isFile==1)
	   	    	my_print(files[i].fileName,strlen(files[i].fileName),1);
		    else
			my_print(files[i].fileName,strlen(files[i].fileName),0);
		    my_print(" ",1,0);
		}
            }
            //printf("\n");
	    my_print("\n",2,0);
            for(int i=0;i<fileNum;i++){
                if(files[i].isFile==0){
                    lsPrint(files[i].fileName);
                }
            }
        }else if(strstr(opr,"ls")!=NULL){
            char* dir=strstr(opr,"/");
            if(dir[0]!='/') {
                //printf("%s%s\n", dir, notAPath);
		my_print(strstr(opr," ")+1,strlen(opr)-3,0);
		my_print(notAPath,strlen(notAPath),0);
                continue;
            }
            char path[20];
            memset(path,0, sizeof(path));
            for(int i=1;i<strlen(dir)-1;i++){
                path[i-1]=dir[i];
            }
            int isDir=0;
            for(int i=0;i<fileNum;i++){
                if(strcmp(files[i].fileName,path)>0&&strstr(files[i].fileName,path)!=NULL&&files[i].isFile==0) {
		    isDir=1;
                    lsPrint(files[i].fileName);
                    memset(path, 0, sizeof(path));
                }
            }
            if(isDir==1){
                continue;
            }else{
                //printf("%s",pathNotFound);
		my_print(pathNotFound,strlen(pathNotFound),0);
            }
        }else if(strstr(opr,"count")!=NULL){
            char* dir=strstr(opr," ");
	    if(dir==NULL){
                //printf("%s",wrgOpe);
		my_print(wrgOpe,strlen(wrgOpe),0);
                continue;
            }
            char path[20];
            memset(path,0, sizeof(path));
            for(int i=1;i<strlen(dir);i++){
                path[i-1]=dir[i];
            }
            int isDir=0;
            for(int i=0;i<fileNum;i++){
                if(strstr(files[i].fileName,path)!=NULL){
                    isDir=1;
                    break;
                }
            }
            if(isDir==0){
		my_print(pathNotFound,strlen(pathNotFound),0);
                //printf("%s",pathNotFound);
                continue;
            }else{
                countDir(path,0);
                for(int i=0;i<fileNum;i++){
                    files[i].isCount=0;
                }
            }
        }else if(strstr(opr,"cat")!=NULL){
            char* dir=strstr(opr,"/");
	    if(dir==NULL){
		my_print(strstr(opr," ")+1,strlen(opr)-3,0);
		my_print(notAFile,strlen(notAFile),0);
		continue;
	    }
            char path[20];
            memset(path,0, sizeof(path));
            for(int i=1;i<strlen(dir);i++){
                path[i-1]=dir[i];
            }
            int isDir=1;
            for(int i=0;i<fileNum;i++){
                if(strcmp(files[i].fileName,path)==0&&files[i].isFile==1){
                    isDir=0;
                    break;
                }
            }
            if(isDir==1){
                //printf("%s%s",path,notAFile);
		my_print(strstr(opr," ")+1,strlen(opr)-3,0);
		my_print(notAFile,strlen(notAFile),0);
		continue;
            }else {
                // char file[20];
                // memset(file,0, sizeof(file));
                // for(int i=1;i<strlen(path);i++){
                //     file[i-1]=path[i];
                //}
                for(int i=0;i<fileNum;i++) {
                    if (strstr(files[i].fileName, path) !=NULL) {
                        catFile(fat,files[i].startClus);
                    }
                }
            }
        }else{
            //printf("%s",wrgOpe);
	    my_print(wrgOpe,strlen(wrgOpe),0);
        }
    }
}

void lsPrint(char dir[]){
    my_print("/",1,0);
    my_print(dir,strlen(dir),0);
    my_print("/:\n",4,0);
    //printf("/%s/:\n",dir);
    for(int i=0;i<fileNum;i++){
        if(strstr(files[i].fileName,dir)!=NULL){
            char path[30];
            memset(path,0, sizeof(path));
            cutStr(path,files[i].fileName,dir);
            if(path[0]!='\0'&&strstr(path,"/")==NULL){
		if(files[i].isFile==1)
                    my_print(path,strlen(path),1);
		else
		    my_print(path,strlen(path),0);
		my_print(" ",1,0);
	    }
            memset(path,0, sizeof(path));
        }
    }
    //printf("\n");
    my_print("\n",2,0);
}

void cutStr(char path[],char name[],char dir[]){
    int i=0;
    for(int j=0;j<70;j++){
        if(name[j]=='\0')
            break;
        if(name[j]==dir[j])
            continue;
        else{
            path[i]=name[j+1];
            i++;
        }
    }
    return;
}

void countDir(char path[],int parent){
    if(parent>0) {
        for (int i = 0; i < parent; i++)
            //printf(" ");
	    my_print("  ",2,0);
    }
    char name[30];
    simply(name,path);
    //printf("%s: ",name);
    my_print(name,strlen(name),0);
    my_print(": ",2,0);
    memset(name,0, sizeof(name));
    int file=0,dir=0;
    for(int i=0;i<fileNum;i++){
        if(strstr(files[i].fileName,path)!=NULL&&strcmp(files[i].fileName,path)>0){
            if(files[i].isFile==1)
                file++;
            else
                dir++;
        }
    }
    //printf("%d file(s), %d dir(s)\n",file,dir);
    if(file>0)
        printNum(file);
    else
	my_print("0",1,0);
    my_print(" file(s), ",10,0);
    if(dir>0)
        printNum(dir);
    else
	my_print("0",1,0);
    my_print(" dir(s)\n",9,0);
    for(int i=0;i<fileNum;i++){
        if(strstr(files[i].fileName,path)!=NULL&&strcmp(files[i].fileName,path)>0){
            if(files[i].isFile==0&&files[i].isCount==0) {
                files[i].isCount=1;
                countDir(files[i].fileName, parent + 1);
            }
        }
    }
}

void printNum(int num){
    if(num>0){
	printNum(num/10);
	int n=num%10;
	char c[1];
	c[0]=n+'0';
	my_print(c,1,0);
	return;
    }
}

void simply(char name[],char path[]){
    int i=0;
    for(int j=0;j<strlen(path);j++){
        if(path[j]=='/')
            i=0;
        else{
            name[i]=path[j];
            i++;
        }
    }
    name[i]='\0';
}

void catFile(FILE* file, int clusNum){
    if(clusNum==0){
	my_print("File is empty!\n",16,0);
    }
    int dataStart=(RsvdSecCnt + NumFATs * FATSz16 + ((RootEntCnt * 32) + (BytsPerSec - 1)) / BytsPerSec) * BytsPerSec;
    char perClus[512];
    while(clusNum<0xFF8){
        fseek(file,dataStart+(clusNum-2)*512,0);
        fread(perClus,1,512,file);
        for(int i=0;i<512;i++){
            if(!perClus[i]=='\0' ){
                //printf("%c",perClus[i]);
		char c[1];
		c[0]=perClus[i];
		my_print(c,1,0);
	    }
            else
                break;
        }
        short re;
        short* read=&re;
        fseek(file,512+clusNum*3/2,0);
        fread(read,1,2,file);
        if(clusNum%2==0){
            clusNum=re&0x0FFF;
        }else {
            clusNum=(re&0xFFF0)>>4;
        }
    }
    //printf("\n");
    my_print("\n",2,0);
}

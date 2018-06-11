#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <math.h>
using namespace std;
struct file_blk{
	struct file_blk *frwd;
	struct file_blk *back;
	char user_data[504];
};
typedef struct file_blk fileblk;

struct dir_blk{
	struct dir_blk *frwd;
	struct dir_blk *back;
    struct unused_blks *free;
	char filler[4];
	char type[31];
	char name[31][9];
	short size[31];	
    union blk *blk_ptr[31];	
};
typedef dir_blk dirblk;

union blk{
	fileblk *fileptr;
	dirblk  *dirptr;
};

struct unused_blks{
	int block_num;
	struct unused_blks * nextblock;
};
typedef struct unused_blks unusedblks;
unusedblks *firstunusedblk;//pointer to the first unused block which points to other unused blocks using linkeds list;
dirblk *currentblk;
dirblk *root;
fileblk *fileptr=NULL;  //used to maintain the file that is currently in handle
int data_ptr=0; // used to handle the data of the currently open file
char mode='-'; //used to maintain the mode of the file that is open currently
int count=0; // unused blocks available at the current state
char file_open[10]; //current file name that is open
int collect=0;
int blocks_required=0;
char dir_in[9]="root";
int write_count=0;

struct folders{
	dirblk *curblk;
	struct folders* next;
};
typedef struct folders path;
path *cur_path;


int stoi(string s){
	int num=0;
	for(int i=0;i<s.length();i++)
	   num=num*10+(s[i]-'0');
	return num;   
}

int ls(){
	dirblk* temp=currentblk;
	int file_count=0;
	while(temp!=NULL){
		dirblk* nextTemp=temp;
		while(nextTemp!=NULL){
			for(int i=0;i<31;i++){
				if(nextTemp->blk_ptr[i]!=NULL){
					file_count++;
					printf("%c\t",nextTemp->type[i]);
					printf("%s\n",nextTemp->name[i]);
				}	
			}
			nextTemp=nextTemp->frwd;
		}
		temp=temp->frwd;	
	}
	return file_count;
}

void init_disk(){
   root=new dirblk; //initialize the root to point to first dir block
   root->back=NULL;
   root->frwd=NULL;
   currentblk=root;                 //current block pointer pointing to root block initially
   cur_path=new path;
   cur_path->curblk=root;
   cur_path->next=NULL;
   firstunusedblk= new unusedblks;
   firstunusedblk->block_num=1;
   unusedblks *temp=firstunusedblk;
   for(int i=2;i<100;i++){
   	    unusedblks *newblk= new unusedblks;
		newblk->block_num=i;
		temp->nextblock=newblk;
		temp=newblk;   	
   }
   root->free=firstunusedblk; //root block is pointing to a linked list of free blocks in a linked list
  return;
}

vector<string> process_input(string input_string){
	stringstream ss(input_string);
	string input;
	vector<string> args;
	while(getline(ss,input,' '))
		args.push_back(input);
	return args;
}

bool exists(char d_f,string f_d_name){
	char fdname[9];
	strcpy(fdname,f_d_name.c_str());
	dirblk *temp=currentblk;
		for(int i=0;i<31;i++){
			if(temp->type[i]==d_f && strcmp(temp->name[i],fdname)==0){
				return true;
			}
		}
	
	return false;	
}

void delete_dir(string dir_name){
	dirblk *temp=currentblk;
	char dirname[9];
	char choice=' ';
	strcpy(dirname,dir_name.c_str());
	if(ls()>0){
		cout <<"Are you sure you want to delete all these files Y/N?\n";
	}
	cin>>choice;
	//choice=tolower(choice);
	cout << choice;
}

void occupy_blocks(){
	count--;
	collect++;
	firstunusedblk=firstunusedblk->nextblock;
}

void collect_free_blocks(){
	unusedblks *temp=firstunusedblk;
	bool arr[100];
	while(temp!=NULL){
		arr[temp->block_num]==1;
	}
	for(int i=1;i<100 && collect>0;i++){
		if(arr[i]==0){
		  unusedblks *newunusedblk=new unusedblks;
		  newunusedblk->block_num=i;
		  newunusedblk->nextblock=firstunusedblk;
		  firstunusedblk=newunusedblk;
	   }
	}	
}

void free_blocks_available(){
	unusedblks *temp=firstunusedblk;
	count=0;
	while(temp!=NULL){
		count++;
		temp=temp->nextblock;
	}
	cout<<"\navailable blocks are "<<count<<"\n";
}

void create_dir(string dir_name){
	char dirName[9];
	strcpy(dirName, dir_name.c_str());
	if(exists('D',dir_name)){
	   cout<<"\ndirectory already exists "<< dir_name<<"\n";
	   dirblk *temp=currentblk;
		while(temp!=NULL){
			for(int i=0;i<31;i++){
				//cout<<" compring files\n";
				if(temp->type[i]=='D' && strcmp(temp->name[i],dirName)==0){
					currentblk=currentblk->blk_ptr[i]->dirptr;
					return;
				}
			}
			temp=temp->frwd;
		}
	}
	else{
		dirblk* cur=currentblk;
		
		ls();
		//cout<<dir_name<<" doesnt exists \n";
		if(firstunusedblk!=NULL){
			int index=-1;
			while(index==-1 && cur!=NULL){
				for(int i=0;i<31;i++){
					if(!(cur->type[i]=='D' || cur->type[i]=='U')){
						index=i;
						break;					
					}
				}
				if(index==-1)
					cur=cur->frwd;
			}
			if(index==-1){
				//allocating one more block for the current_block. Using *frwd
				unusedblks * temp= firstunusedblk;
				
				cur=currentblk;
				while(cur->frwd!=NULL){
					cur=cur->frwd;
				}
				if(temp!=NULL){	
					cur->frwd=new dirblk;
					cur->frwd->back=cur;
					cur=cur->frwd;
					cur->frwd=NULL;
					occupy_blocks();
					index=0;	
				}
				else{
					cout<< " No space to create new directory..";
					return;
				}
			}
			dirblk* temp=new dirblk;
			temp->back=NULL;
			temp->frwd=NULL;
			occupy_blocks();
			cur->blk_ptr[index]=new blk;
			cur->blk_ptr[index]->dirptr=temp;
			strcpy(cur->name[index],dirName);
			cur->type[index]='D';
		}
	}  	
}

void change_directory(string dir_name){
	char dirName[9];
	if(dir_name==".."){
		
		if(cur_path->next!=NULL){
		 currentblk=cur_path->curblk;
		 cur_path=cur_path->next;
	    }
	    return;
	}
	strcpy(dirName,dir_name.c_str());
	for(int i=0;i<31;i++){
		if(currentblk->type[i]=='D' && strcmp(currentblk->name[i],dirName)==0){
			path *temp=new path;
			temp->curblk=currentblk;
			temp->next=cur_path;
			cur_path=temp;
			currentblk=currentblk->blk_ptr[i]->dirptr;
			cout<< "\nchanged directory to "<<dir_name<<"\n";
		}
	}
} 

void open_file(string f_name){
	char fname[9];
	strcpy(fname,f_name.c_str());
	data_ptr=0;
	for(int i=0;i<31;i++){
		if(currentblk->type[i]=='U' && strcmp(currentblk->name[i],fname)==0){
			strcpy(file_open,fname);
			fileptr=currentblk->blk_ptr[i]->fileptr;
			return;
		}
	}
	if(fileptr==NULL){
	  cout<<"\n could not find file "<<f_name;
	  cout<<"\nlist of files in this directory are\n";
	  ls();
	  return;
	}
}

void delete_file(){
	dirblk *cur=currentblk;
	while(cur!=NULL){
		for(int i=0;i<31;i++){
			//cout<<" compring files\n";
			if(cur->type[i]=='U' && cur->blk_ptr[i]->fileptr==fileptr){
					cur->blk_ptr[i]=NULL;
					cur->type[i]='-';
					while(fileptr!=NULL){
					  fileptr=fileptr->frwd;
					  occupy_blocks();
					}
					collect_free_blocks();
					cout<<" \ndeleted file"<<"\n";	
					return;
			}
		}
		cur=cur->frwd;
	}
	if(cur==NULL)
	   cout<<"\ncould not find the fileto delete \n";
}

void create_file(string file_name){
	char fileName[9];
	strcpy(fileName, file_name.c_str());
	//cout<< "creating file "<<file_name;
	if(exists('U',file_name)){
	  //delete_file();
	  //create_file(file_name);
	  cout<"file exists";	
	}
	else{
		dirblk *cur=currentblk;
		if(firstunusedblk!=NULL){
			int index=-1;
			while(cur!=NULL && index==-1){
				for(int i=0;i<31;i++){
					if(cur->blk_ptr[i]==NULL){
					  index=i;
					  break;
					}
				}
				if(index==-1)
				  cur=cur->frwd;
			}
			if(index==-1){
				cur=currentblk;
				while(cur->frwd!=NULL)
				 cur=cur->frwd;
				dirblk *temp=new dirblk;
				temp->back=cur;
				cur->frwd=temp;
				occupy_blocks();
				cur=temp; 	
			}
			cur->blk_ptr[index]=new blk;
			cur->blk_ptr[index]->fileptr=new fileblk;
			cur->blk_ptr[index]->fileptr->frwd=NULL;
			cur->blk_ptr[index]->fileptr->back=NULL;
			strcpy(cur->name[index],fileName);
			occupy_blocks();
			cur->type[index]='U';	
			cout <<"new file created \n";
		}
		else{
			cout<< "\n No disk space available\n";
		}
	}
}

void create(vector<string> args){
   //cout<<" creating arguments\t"<< args[1];
   
   stringstream ss(args[2]);
   string name;
   int dircount=0;
   
   for(int i=0;i<args[2].length();i++){
       if(args[2][i]=='/')
           dircount++;
   }
   char temp_dir[9];
   strcpy(temp_dir,dir_in);
   while(dircount!=0 && getline(ss,name,'/')){
	 create_dir(name);
	 change_directory(name);
	 strcpy(dir_in,name.c_str());
	 dircount--;	  
   }
   
   getline(ss,name,'/');
   //cout<<"file name is "<<name;
   if(args[1]=="U"){
	 create_file(name);
     open_file(name);
   }
   else{
      create_dir(name);
      change_directory(name);
    }
   strcpy(dir_in,name.c_str());
   return;
}

void close_file(){
  fileptr=NULL;
  mode='-';
  data_ptr=0;
}

void reset(){
	while(fileptr->back!=NULL)
	  fileptr=fileptr->back;
	data_ptr=0;
}

void read_data(int bytes){
	cout <<"\n file contents \n";
	if(fileptr!=NULL){
		fileblk *temp=fileptr;
		cout<<" reading data\n";
		while(temp!=NULL && bytes>0){
			char data[504]={' '};
			int readbytes=min(bytes,504);
			bytes-=504;
			//cout << "\n reading bytes --  "<<readbytes<<"\n";
			strncpy(data,temp->user_data+data_ptr,readbytes);
			data_ptr+=readbytes;
			data_ptr=data_ptr%504;
			data[readbytes]='\0';
			cout<<data;
			temp=temp->frwd;
		}
		cout <<"\n";
	}
	else
	 cout<< "no file is open to perform read operation\n";
}

void write(char data[504],int bytestoread){
    cout<< "\nwriting to file\n";
    //int bytestoread=stoi(bytes);
	if(fileptr!=NULL && (mode=='U' || mode=='O')){
    	strncpy(fileptr->user_data+data_ptr,data,bytestoread);
    	//data_ptr+=504;
    	cout<< "\ndata written\n";
	}	
}

void seek(int index){
	cout<< "index is "<<index<<"\n";
    int initptr=data_ptr;
	if(abs(data_ptr-initptr)!=abs(index)){
		if(index<0){
			cout << "inside index < 0\n";
			int blks_to_move=floor((double)abs(index))/504;
			while(fileptr->back!=NULL){
				fileptr=fileptr->back;
			}
			cout << " blocks to be moved "<<blks_to_move<<"\n";
			data_ptr-=(blks_to_move*504);
			data_ptr-=abs(index)-abs(data_ptr-initptr);
			cout <<" the difference is "<<abs(data_ptr-initptr)<<"\n";
			if(data_ptr<0)
			  data_ptr=0;
		}
		else{
			int blks_to_move=floor((double)abs(index))/504;
			while(fileptr->frwd!=NULL){
				fileptr=fileptr->frwd;
			}
			data_ptr+=(blks_to_move*504);
			data_ptr+=abs(index)-abs(data_ptr-initptr);
			if(data_ptr>504)
			  data_ptr=503;
		}
	} 
	data_ptr=data_ptr%504;
}

void process_command(vector<string> args){
	if(args[0]=="create"){
		if(args.size()==3){
			
			create(args);
			//currentblk=temp;
			//ls();
		}
		else
		    cout<<"\nusage: create U/D filepath/dirpath";	
	}
	else if(args[0]=="cd"){
	//	cout<< "\nchanging directory\n";
		change_directory(args[1]);
	}
	else if(args[0]=="open"){
		if(args.size()==3){
			mode=args[1][0];
			if(mode=='I' || mode=='O' || mode=='U')
			   open_file(args[2]);
			else
			   cout<<"usage: open I/O/U filepath";   
		}		
		else
		    cout<<"usage: open I/O/U filepath";	
	}
	else if(args[0]=="write"){
	//	cout<<"\nwrtiting\n";
		if(args.size()==3){
			int n=stoi(args[1]);
            
			if(mode=='O' || mode=='U'){
				int index=0;
				blocks_required=ceil((double)n/504);
				free_blocks_available();
				cout << "blocks required for the file --"<<blocks_required<<"\n";
				if(count<blocks_required-1){
				  cout<< "\nspace unavilable to insert data of size "<<n<<"\n";	
				  cout<< "space available is "<<(count*504)<<" bytes\n";
				  return;
				}
				fileblk *prev=fileptr;
				while(blocks_required!=0){
				   fileblk *temp=new fileblk;
				   temp->frwd=NULL;
				   prev->frwd=temp;
				   temp->back=prev;
				   prev=temp;
				   blocks_required--;
				   occupy_blocks();
				}
				
				while(n>0){
					char buffer[504];
					if(n>504){
						buffer[504]={'-'};
						cout<<" in first block\n";
						int read=min(504,504-data_ptr);
						if(index<args[2].size())
							args[2].copy(buffer,read,index);
						write(buffer,504);
						index+=read;
						n-=read;
						fileptr=fileptr->frwd;
					}
					else{
						buffer[504]={'-'};
						cout<<" int his block error\n";
						int occupied=0;
						if(args[2].size()>index)
							occupied=args[2].copy(buffer,504,index);
						for(int i=occupied;i<504;i++)
						   buffer[i]=' ';
						write(buffer,n);
						n=0;   
					}
				}
			}
			else{
				cout<<"\nfile is not open in write/Update mode\n";
				return;
			}
		}
		else{
			cout <<"usage: write n 'data'";
		}
	}
	else if(args[0]=="ls"){
		ls();
	}
	else if(args[0]=="read"){
		int bytes=stoi(args[1]);
		//string res=args[0].substr(1,bytes);
		//args[1]=res;
		cout <<" buytes "<< bytes;
		read_data(bytes);
	}
	else if(args[0]=="delete"){
		delete_file();
	}
	else if(args[0]=="seek"){
		//need to implement
		int index=1;
		if(args[1][0]=='-'){
		  args[1][0]='0';
		  index=-1;
	    }   
		index=index*stoi(args[1]);
		seek(index);
	}
	else if(args[0]=="reset"){
		reset();
		cout<< "file pointer is reset";
	}
	else if(args[0]=="rmdir"){
		//delete_dir("sample");
	}
	else{
		cout << ":( invalid command\n";
	}
}

int main(){
   	init_disk();
   	
    string input_string;
    vector<string> args;
    
    printf("file blk %d",sizeof(file_blk));
	printf("--dir blk %d",sizeof(dirblk));
	cout<<"\n";
	
    do{
    	collect=0;
    	printf("Enter the command:\n");
	    getline(cin,input_string);
        args=process_input(input_string);
		process_command(args);
		free_blocks_available();
	}while(args[0]!="exit");	
}


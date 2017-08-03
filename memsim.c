
/*program implementing VMS 2ndchance and lru algorithms using circular single linked list*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
int nframes=0;
char* method;
char* mode;
char* tracefile;
int hits=0,misses=0,diskwrites=0,diskreads=0,pagefaults=0;
int capacity=1;
bool first = 1;
int dbg = 0;
struct frame
{
unsigned page;
int nwrites;
char ch2;
struct frame *next;
};
struct frame *start_frame=NULL,*last,*new_frame;

void createvmsLL(unsigned pagenum,int dirty)
{
	//CAlled for VMS..Creates a linked list inserting all the frames untill nframes
	new_frame = (struct frame *)malloc(sizeof(struct frame));
	new_frame->page = pagenum;
	new_frame->nwrites = dirty;
	new_frame->ch2 = '0';
	if(start_frame == NULL)
	{
		//Creating first node
		if(dbg==1)
		printf("Creating new node");
		new_frame->next = NULL;
		start_frame = new_frame;
		last = start_frame;
		capacity++;
	}
	else//if node is already present
	{
		//Attaching frame in the end and point last to first_frame
		last->next=new_frame;
		last = new_frame;
		last->next=start_frame;
	}
	diskreads++;//Every new frame creation is diskread/hence increasing the counter
}
void createLL(unsigned pagenum,int dirty)
{
	//Function called for LRU..Creates a linked list inserting all the nodes untill nframes
	struct frame *temp;
	new_frame = (struct frame *)malloc(sizeof(struct frame));
	new_frame->page = pagenum;
	new_frame->nwrites = dirty;
	if(start_frame == NULL)
	{
		//Creating first node
		if(dbg==1)
		printf("Creating new node");
		new_frame->next = NULL;
		start_frame = new_frame;
		last = start_frame;
		capacity++;
	}
	else//if a node is already present
	{
		//Attach the new node in the beginning
		//make it start_frame
		new_frame->next = start_frame;	
		start_frame = new_frame;
		last->next=start_frame;
	}
	diskreads++;//Every new frame creation is diskread/hence increasing the counter
}

void displayLL()
{
	//Function to display memory instance after each page reference
	//Called only when debug is on
	struct frame *temp;
	temp = start_frame;
	printf("\nPrinting Memory instance \n");
	do
    {
		printf("%x %d %c\n",temp->page,temp->nwrites,temp->ch2);
		temp=temp->next;
    }while(temp!=start_frame && temp!=NULL);
	printf("*** Memory instance End ***\n");
}


struct frame* findpage(unsigned pagenum)
{
	//Checks for page in List and returns page/NULL
	struct frame *temp = start_frame;
	struct frame *current=NULL;
	do
	{
		if(temp->page == pagenum)
		{
			current = temp;
			break;
		}
		else
			temp=temp->next;
	}while(temp!=start_frame && temp!= NULL); //do not loop if 1 traverse is completed or no node
	return current;
}
void implementVMS(unsigned pagenum,int dirty)
{
	struct frame *previouspage,*currentpage,*temp,*ptr;
	if(first)//Checking if it is the first frame
	{
		createvmsLL(pagenum, dirty);
		first = 0;
		ptr=start_frame;
		misses++;
		if(dirty==1)
			diskwrites++;
	}
	else//If list has been created already
	{
		currentpage = findpage(pagenum);//Calling function to check if the page is already in memory
		if(currentpage!=NULL && start_frame!= NULL)
		{
			//if page has been found,updating it chance 2 bit and read/write character
			if(currentpage->nwrites==0 && dirty==1)
			{
				diskwrites++;
			}
			currentpage->ch2 = '1';
			currentpage->nwrites = dirty;
			//currentpage->mod=op;
			hits++;
		}
		else
		{ 
			//Reaches here if page is not present in memory
			misses++;
			if(dbg==1)
			printf("size: %d", capacity);
			if(capacity <= nframes)//If space is available directly insert
			{
				capacity++;
				createvmsLL(pagenum,dirty);
			}
			else //remove first encountered ch2=0 page and then insert
			{
				pagefaults++;
				for(temp = ptr;temp->ch2!='0';temp=temp->next)//traversing untill chance2 bit=0 is encountered
				{
					temp->ch2 ='0';
				}
				//replace temp with new address
				if(dbg==1)
				printf("\n Replacing page: %x %d %s",temp->page,temp->nwrites,temp->next);
				temp->page = pagenum;
				//temp->mod=op;
				temp->nwrites = dirty;
				temp->ch2='0';
				ptr = temp->next;
				diskreads++; //increasing counter since page has to be read
			}
			if(dirty==1)
				diskwrites++;
		}
	}
	if(dbg==1)
	displayLL();
}
void implementLRU(unsigned pagenum,int dirty)
{
	struct frame *previouspage,*currentpage,*temp;
	if(first)//Checking if it is the first frame
	{
		createLL(pagenum, dirty);
		first = 0;
		misses++;	
		if(dirty==1)
			diskwrites++;
	}
	else
	{
		currentpage = findpage(pagenum);//Calling function to check if the page is already in memory
		
		if(currentpage!=NULL && start_frame!= NULL)
		{
			//if page is already present,move it to the beginning
			if(currentpage->nwrites==0 && dirty==1)
			{
				diskwrites++;
			}
			currentpage->nwrites=dirty;
			hits++;
			if(currentpage != start_frame)
			{
				//try to move to front only if it is not first node
				for(temp = start_frame;temp->next!=currentpage;temp=temp->next);
				previouspage = temp;
				if(currentpage->next==start_frame)
				last = previouspage;
				previouspage->next = currentpage->next;
				currentpage->next = start_frame;
				start_frame=currentpage;
				last->next=start_frame;
			}			
		}
		else//Page not present in memory insert
		{
			misses++;
			if(dbg==1)
			printf("size: %d", capacity);
			if(capacity <= nframes)//If space is available directly insert
			{
				createLL(pagenum,dirty);
				capacity++;
			}
			else // Page not present remove page by selecting victim through lru
			{
				pagefaults++;
				//removing from rear end
				for(temp = start_frame;temp->next->next!=start_frame;temp=temp->next); //loop untill last but one
				previouspage = temp;
				currentpage = previouspage->next;
				//if(currentpage->mod == 'W')
					//diskwrites++;
				previouspage->next = start_frame;
				last = previouspage;
				if(dbg==1)
				printf("Deleting %x",currentpage->page);
				free(currentpage);
				createLL(pagenum,dirty);
			}
			if(dirty==1)
				diskwrites++;
		}
	}
	//print List
	if(dbg==1)
	displayLL();
}
void main(int argc,char*argv[])
{
	unsigned lineaddres,pagenum;
	char rw;
	char alg[3];
	int ret;
	char r;
	int events=0;
	int dirty=0;
	tracefile = argv[1];
	nframes = atoi(argv[2]);
	method = argv[3];
	mode = argv[4];
	FILE *myfile;
	myfile = fopen(tracefile,"r"); //Opening inputted file
	r=fscanf(myfile,"%x %c",&lineaddres,&rw);
	strcpy(alg,"vms");
	ret = strcmp(method,alg);
	if(strcmp(mode,"debug")==0)
		dbg =1;
	pid_t pid = getpid();
	while(r!=EOF)
	{
		dirty=0;
		events++;
		pagenum = lineaddres>>12;
		if(dbg==1)
		{
			printf("Memory address: %x %c \n",lineaddres,rw);
			printf("Pagenumber is: %x \n",pagenum);
		}
		//Checking if a page is dirty
		if(rw == 'W')
		{
			dirty=1;
		}
		//Checking the inputted method and calling the same.
		if(ret == 0)
			implementVMS(pagenum,dirty);
		else 
			implementLRU(pagenum,dirty);	
		r=fscanf(myfile,"%x %c",&lineaddres,&rw);
	}
	fclose(myfile);
	//Printing the output
	//printf("%d",sizeof(struct frame));
	//printf("%d %d",sizeof(unsigned),sizeof(int));
	//printf("pid: %d", pid);
	printf("\n Total number of memory frame:%d",nframes);
	printf("\n Total number of events:%d",events);
	//printf("\n Number of hits:%d", hits);
	//printf("\n Number of misses:%d", misses);
	printf("\n Number of diskwrites:%d", diskwrites);
	printf("\n Number of diskreads:%d", diskreads);
	//printf("\n Number of pagefaults:%d", misses);
}

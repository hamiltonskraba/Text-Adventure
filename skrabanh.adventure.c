#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>

//define constants
#define MAXCONNECT 6
#define MINCONNECT 3
#define NUMROOMS 7
#define TRUE 1
#define FALSE 0

typedef int bool;

char userEntry[10];
char folderName[21];
char firstRoom[10];
char roomPath[500];
int stepCount = 0;
pthread_mutex_t mutex;

//returns void pointer to start point for time thread
void* writeTimeFile() {
	char str[80];								//just some variables to hold stuff
	time_t timeNow;
	struct tm *localTime;
	FILE* timeFile;

	time(&timeNow);
	localTime = gmtime(&timeNow);				//get and convert current time to time "object"

	timeFile = fopen("currentTime.txt", "w+");						//create a file for writing and reading
	strftime(str, 80, "%I:%M%p, %A, %B %d, %Y", localTime);			//format string

	fputs(str, timeFile);		
	fclose(timeFile);
	return NULL;
}


//prints the time to the console
void printTheTime() {
	char str[80];
	FILE* readFile;

	readFile = fopen("currentTime.txt", "r");			//open the file
	fgets(str, 80, readFile);							//read the time string
	printf("%s\n\n", str);								//print it to the console
	fclose(readFile);
}

void manageThreads() {
	pthread_t newThread;			 //create new thread
	pthread_mutex_lock(&mutex);		 //lock currnt thread
	
	int check = pthread_create(&newThread, NULL, writeTimeFile, NULL);	//run new thread
	pthread_mutex_unlock(&mutex);		//once done unlock old thread
	pthread_join(newThread, NULL);		//join

}

//returns the most recently built set of rooms in the rooms directory
//copied and slightly modified from the lecture
void getMostRecentDirectory()
{
	int newestDirTime = -1; // Modified timestamp of newest subdir examined
	char targetDirPrefix[32] = "skrabanh.rooms."; // Prefix we're looking for
	char newestDirName[256]; // Holds the name of the newest dir that contains prefix
	memset(newestDirName, '\0', sizeof(newestDirName));

	DIR* dirToCheck; // Holds the directory we're starting in
	struct dirent *fileInDir; // Holds the current subdir of the starting dir
	struct stat dirAttributes; // Holds information we've gained about subdir

	dirToCheck = opendir("."); // Open up the directory this program was run in

	if (dirToCheck > 0) // Make sure the current directory could be opened
	{
		while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
		{
			if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
			{
				//debug printf("Found the prefex: %s\n", fileInDir->d_name);
				stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

				if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
				{
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);
					//debug printf("Newer subdir: %s, new time: %d\n",
						//fileInDir->d_name, newestDirTime);
					strcpy(folderName, fileInDir->d_name);
				}
			}
		}
	}

	closedir(dirToCheck); // Close the directory we opened
}


//searches through files in rooms directory to return the name of the start room
void findStartRoom() {
	DIR* folder;
	struct dirent* file;
	FILE *readFile;
	char buffer[9];//8 for no terminator, 9 for with
	char line[40];
	char* roomType = "ROOM TYPE: START_ROOM\n";
	
		folder = opendir(folderName);					//set directory to be read as folder

		file = readdir(folder);
		chdir(folderName);

		while ((file = readdir(folder)) != NULL){				//while there are files to be read

			if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")) {	//These paths might be read as directories
				continue;							//but they're not they files we're looking for
			}

			strcpy(buffer, file->d_name);				//copy the file name to a variable

			readFile = fopen(buffer, "r");				//open the current file for reading

			while (fgets(line, 40, readFile) != NULL) {		//read file until reaching room type
				if (strcmp(line, roomType) == 0) {
					strcpy(firstRoom, buffer);		//save the start room name when matching string
					break;
				}
			}

			fclose(readFile);
		}
	

	closedir(folder);
}

//writes a room's title and connections to the console
void getRoomData(char name[10]) {
	FILE *readFile;							//dummy variables and pointers for reading
	char line[40];
	int connectionCount;
	char word[11], colon [3], title[11];

	int count = getConnectionCount(name);	//count the number of connections for formatting purposes
	int printCount = 0;

	printf("CURRENT LOCATION: %s\n", name);
	printf("POSSIBLE CONNECTIONS: ");

	readFile = fopen(name, "r");			//print name of current room
	
	fgets(line, 40, readFile);				//skip first line containing title

	//read connections---------------------------
	fgets(line, 40, readFile);

	sscanf(line, "%s %d %s %s", word, &connectionCount, colon, title);

	while (strcmp(word, "CONNECTION") == 0) {		//while first string token is a connection
		if (printCount < count - 1) {				//format printing
			printf("%s, ", title);
		}
		else {
			printf("%s", title);
		}
		printCount++;
		
		fgets(line, 40, readFile);					//get next line
		sscanf(line, "%s %d %s %s", word, &connectionCount, colon, title);
	}
	printf(".\n");
}

//returns the number of connections for a room
int getConnectionCount(char *name) {
	FILE *readFile;							//dummy variables and pointers for reading
	char line[40];
	char word[11], colon[3], title[11];
	int connectionCount;

	int count = 0;							//actual count

	readFile = fopen(name, "r");

	fgets(line, 40, readFile);				//skip first line containing

	//read connections---------------------------
	fgets(line, 40, readFile);

	sscanf(line, "%s %d %s %s", word, &connectionCount, colon, title);

	while (strcmp(word, "CONNECTION") == 0) {			//while first string token is a connection
		count++;										//iterate count
		fgets(line, 40, readFile);
		sscanf(line, "%s %d %s %s", word, &connectionCount, colon, title);
	}
	
	return count;
}

//checks that a user's entry is either a valid connection, or time request
void checkRoomConnection(char user[10], char room[10]) {
	FILE *readFile;							//dummy variables and pointers for reading
	char line[40];
	int connectionCount;
	char word[11], colon[3], title[11];

	bool success = FALSE;

	readFile = fopen(room, "r");

	fgets(line, 40, readFile);				//skip first line for title

	//read connections---------------------------
	fgets(line, 40, readFile);

	sscanf(line, "%s %d %s %s", word, &connectionCount, colon, title);

	while (strcmp(word, "CONNECTION") == 0) {		//while the first string token is a connection
		if (strcmp(user, title) == 0) {				//test that the user entry is a valid connection
			success = TRUE;
			strcpy(firstRoom, user);				//move to the new connection
			stepCount++;
			strcat(roomPath, user);
			strcat(roomPath, "\n");
		}
		fgets(line, 40, readFile);
		sscanf(line, "%s %d %s %s", word, &connectionCount, colon, title);
	}

	if(strcmp(user, "time") == 0){			//if user wants time
	manageThreads();				//lock current thread and make time file
	printTheTime();					//read and print tie file
	success = TRUE;
	}

	if (!success) {									//prompt user for another entry if invalid
		printf("HUH? I DON\'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
	}
}

//checks to see if the current room is the end room
bool checkForW(char room[10]) {
	FILE *readFile;
	char line[40];
	int connectionCount;
	char word[11], colon[20], title[11], type[11];
	char test[20] = "CONNECTION";
	char test2[20] = "END_ROOM";

	readFile = fopen(room, "r");

	fgets(line, 40, readFile);				//skip first line for title

	fgets(line, 40, readFile);				//ignore all connections
	sscanf(line, "%s %d %s %s", word, &connectionCount, colon, title);

	while (strcmp(word, "CONNECTION") == 0) {
		fgets(line, 40, readFile);
		sscanf(line, "%s %d %s %s", word, &connectionCount, colon, title);
	}

	fgets(line, 40, readFile);

	sscanf(line, "%s %s %s", word, colon, type);
	if (strcmp(type, "END_ROOM") == 0) {			//check for the end room
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void getUserInput() {
	fgets(userEntry, 20, stdin);
	//cut off newline from user entry
	//referenced stackoverflow.com/questions/27491005/how-can-i-get-a-string-from-input-without-including-a-newline-using-fgets
	size_t len = strlen(userEntry);						//get the length of the string
	if (len > 0 && userEntry[len - 1] == '\n') {		//if the last char is a newline, change it to a null terminator
		userEntry[--len] = '\0';
	}
}


void main() {
	int result = FALSE;

	getMostRecentDirectory();

	findStartRoom();

	getRoomData(firstRoom);

	while (result != TRUE) {

		printf("WHERE TO? >");
		getUserInput();
		printf("\n");

		checkRoomConnection(userEntry, firstRoom);

		result = checkForW(firstRoom);

		if (result) {
			break;
		}

		getRoomData(firstRoom);
	}
	printf("YOU HAVE FOUND THE END ROOM.CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCount);
	printf(roomPath);

	exit(0);

}






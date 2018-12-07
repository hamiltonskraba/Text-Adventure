#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/stat.h> 

//define constants
#define MAXCONNECT 6
#define MINCONNECT 3
#define NUMROOMS 7
#define TRUE 1
#define FALSE 0

typedef int bool;

struct room
{
	int id;
	char* title;
	char* roomType;
	int connectionCount;
	int connections[MAXCONNECT];
};

char* roomNames[10] = { "ummaguma", "atomhart", "meddle", "darkside", "wishywh", "animals", "thewall", "fincut", "momlapse", "divbell" };
char* roomTypes[3] = { "START_ROOM", "MID_ROOM", "END_ROOM" };
char folderName[30];

//returns a random integer 0 - 9
int getRandomTen() {
	int rando;
	rando = rand() % 10;
	return rando;
}

//returns a random integer 0 - 6
int getRandomSeven() {
	int rando;
	rando = rand() % NUMROOMS;
	return rando;
}

//returns a pointer to a random room from the list
struct room getRandomRoom(struct room roomList[]) {
	int rando = getRandomSeven();
	struct room *tempRoom = &roomList[rando];
	return *tempRoom;
}

//checks that a room doesn't have more than 6 connections
bool canAddConnectionFrom(struct room room)
{
	if (room.connectionCount <= MAXCONNECT) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

//checks to make sure 2 random rooms are not the same
bool isSameRoom(struct room roomA, struct room roomB) {
	if (roomA.id == roomB.id) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

//checks if a room's id is already held in the connections array
bool connectionAlreadyExists(struct room roomA, struct room roomB) {
	int i;
	for ( i = 0; i < MAXCONNECT; i++) {
		if (roomA.connections[i] == roomB.id) {
			return TRUE;
		}
	}
	return FALSE;
}

//places a room's id into the next available slot in the connections array and iterates count
void connectRooms(struct room *roomA, struct room *roomB) {
	int i;
	for (i = 0; i < MAXCONNECT; i++) {
		if (roomA->connections[i] == -1) {
			roomA->connections[i] = roomB->id;
			roomA->connectionCount++;
			break;
		} 
	}
}

//makes a connection between two rooms given if all other conditions are satisfied
//Connection function arcitechture and headings borrowed from lecture 
void addRandomConnection(struct room roomList[])
{
	struct room A;
	struct room B;

	while (TRUE)
	{
		A = getRandomRoom(roomList);
		if (canAddConnectionFrom(A) == TRUE)
			break;
	}

	do
	{
		B = getRandomRoom(roomList);
	} while (canAddConnectionFrom(B) == FALSE || isSameRoom(A, B) == TRUE || connectionAlreadyExists(A, B) == TRUE);

	connectRooms(&A, &B);  
	roomList[A.id] = A;

	connectRooms(&B, &A);
	roomList[B.id] = B;

	//debug printf("connecting room %d and %d\n", A.id, B.id);
}

//checks to see that every room has at least 3 connections
bool isGraphFull(struct room roomList[])
{
	int i;
	for (i = 0; i < MAXCONNECT; i++) {
		if (roomList[i].connectionCount < MINCONNECT) {
			return FALSE;
		}
	}
	//debug printf("graph is full now \n");
	return TRUE;
}

//makes a folder and transers in all room data as seperate files
void transferRooms(struct room roomList[], char *folderName) {
	chdir(folderName);											//move to proper directory

	int i;
	for (i = 0; i < NUMROOMS; i++) {
		FILE* current = fopen(roomList[i].title, "a");			//make a new file for first room in append mode
		fprintf(current, "ROOM NAME: %s\n", roomList[i].title);	//append room name

		int j;
		for (j = 0; j < roomList[i].connectionCount; j++) {			//for every connection in current room
			fprintf(current, "CONNECTION %d: %s\n", j + 1, roomList[roomList[i].connections[j]].title); //append connection count and index's title
		}

		fprintf(current, "ROOM TYPE: %s\n", roomList[i].roomType);	//append room type

		fclose(current);											//close file, because we're responsible adults
	}
}

void main() {

	struct room roomList[NUMROOMS];
	int takenRooms[NUMROOMS];

	int roomId = 0;

	//seed random function
	srand(time(NULL));

//start room-------------------------------------------------------------
	roomList[0].id = roomId;					//set first id

	int rando = getRandomTen();					//generate random int 0 - 9

	takenRooms[roomId] = rando;					//place index in a list of taken indeces

	roomList[0].title = roomNames[rando];		//set title, count, and type
	roomList[0].connectionCount = 0;
	roomList[0].roomType = roomTypes[0];		//set type

	roomId++;											//next room
	
//mid rooms-----------------------------------------------------------------------
	int i, j;
	for (i = 1; i < 6; i++) {					//for the next 5 rooms
		roomList[i].id = roomId;				//set id

		rando = getRandomTen();					//random int 0 - 9

		for (j = 0; j < 7; j++) {
			if (rando == takenRooms[j]) {		//check that random room isn't already chosen
				rando = getRandomTen();			//get a new number
				j = -1;							//reset count (it gets iterated at the top of the loop!)
			}
		}

		takenRooms[roomId] = rando;				//push next room onto chosen list

		roomList[i].title = roomNames[rando];
		roomList[i].connectionCount = 0;
		roomList[i].roomType = roomTypes[1];	//set type
		roomId++;								//next room 
	}

	//last room---------------------------------------------------------------------
	roomList[6].id = roomId;
	rando = getRandomTen();

	for (j = 0; j < roomId; j++) {
		if (rando == takenRooms[j]) {
			rando = getRandomTen();
			j = -1;
		}
	}

	takenRooms[roomId] = rando;

	roomList[i].title = roomNames[rando];
	roomList[i].connectionCount = 0;
	roomList[i].roomType = roomTypes[2];

	//set all connections to -1
	for (i = 0; i < NUMROOMS; i++) {
		for (j = 0; j < MAXCONNECT; j++) {
			roomList[i].connections[j] = -1;
		}
	}

	//make all connections between rooms
	while (isGraphFull(roomList) == FALSE) {
		addRandomConnection(roomList);
	}
	
	//print room info for debugging-------------------------------------------------------------------------
	//int k;
	//for (k = 0; k < NUMROOMS; k++) {
	//	printf("Room ID #%d %s %s \n", roomList[k].id, roomList[k].title, roomList[k].roomType);
	//}

	//make folder and store contents------------------------------------------------------------------------
	int pid = getpid();								//get pid for folder name

	char folderName[30];								//build folder name
	sprintf(folderName, "skrabanh.rooms.%d", pid);
	mkdir(folderName, 0700);							//make new folder with proper permissions

	transferRooms(roomList, folderName);				//transfer data into folder as a seperate file for each room
	
}

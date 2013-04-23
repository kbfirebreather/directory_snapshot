#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>

const char * DEFAULT_DIR_ROOT = "/media/pogostick/Television";
const char * DEFAULT_OUT_ROOT = "/home/pogostick/directory_snapshot/";
const char * DEFAULT_OUT_FILE = "snapshot";

char dir_root[1000];
char out_root[1000];
char out_file[1000];

struct folders {
	int is_folder;
	char path[1000];
	struct folders *next;
	//struct folders *prev;
};

int print_to_file(struct folders *folders_ptr);
int print_list(struct folders * folders_ptr);
int print_folder_list(struct folders * folders_ptr);
int print_file_list(struct folders *folders_ptr);
int sort_list(struct folders * folders_ptr);
int append_folder_list(struct folders * folders_ptr, char path[1000], unsigned char d_type);
int populate_directory(struct folders * folders_ptr, char * directory);
void display_readme();
void fix_out_root();
void fix_dir_root();
char * format_output_file();
char * maketime();


int print_to_file(struct folders *folders_ptr)
{
	char * output_file = format_output_file();

	FILE *file; 
	file = fopen(output_file, "w");
	if(file == NULL)
	{
		printf("Unable to open output file for writting: %s\n", output_file);
		exit(0);
	}

	free(output_file);

	struct folders *conductor; // traverse linked list
	conductor = folders_ptr; // point to beginning of linked list
	while(conductor->next != NULL) // will take us to second to last node
	{
		fprintf(file, "%s\n", conductor->path); // print to file
		conductor = conductor->next; // on to the next node
	}
	fprintf(file, "%s\n", conductor->path); // print last node to the file

	fclose(file);
}

// print everything to screen
int print_list(struct folders * folders_ptr)
{
	struct folders *folders_t;
	folders_t = folders_ptr;
	while(folders_t->next != NULL)
	{
		printf("type: %d -- Folder: %s\n", folders_t->is_folder, folders_t->path);
		folders_t = folders_t->next;
	}
	printf("type: %d -- Folder: %s\n", folders_t->is_folder, folders_t->path); // last node
}

// print folders only to screen
int print_folder_list(struct folders * folders_ptr)
{
	struct folders *folders_t;
	folders_t = folders_ptr;
	while(folders_t->next != NULL)
	{
		//printf("Folder_bit: %i\n", folders_t->is_folder);
		if(folders_t->is_folder == 1)
			printf("Folder: %s\n", folders_t->path);

		folders_t = folders_t->next;
	}
	printf("Folder: %s\n", folders_t->path); // last node
}

// print files only to screen
int print_file_list(struct folders *folders_ptr)
{
	struct folders *folders_t;
	folders_t = folders_ptr;
	while(folders_t->next != NULL)
	{
		if(folders_t->is_folder == 0)
			printf("Folder: %s\n", folders_t->path);
		folders_t = folders_t->next;
	}
	printf("Folder: %s\n", folders_t->path); // last node
}

/* Function: sort_list
 *
 * Purpose: sorts linked list in alphabetical order on ->path using bubble sort
 *
 * @params: @folders_ptr - ptr to struct
 *
 * Return: 
 *
 * Note: 
 *
*/
int sort_list(struct folders * folders_ptr)
{
	struct folders * folders_t;
	folders_t = folders_ptr; // traverse list
	int switched = 1;
	while(switched == 1) // keep going until an entire pass occurs without a swap
	{
		switched = 0; // reset state
		folders_t = folders_ptr; // point to beginning of list
		while(folders_t->next != NULL) // iterate to the n-1 node
		{
			char path[1000]; // local
			char path2[1000]; // local
			int is_folder1 = folders_t->is_folder; // current nodes state
			int is_folder2= folders_t->next->is_folder; // next nodes state
			strcpy(path,folders_t->path); // save current nodes path
			strcpy(path2,folders_t->next->path); // save next nodes path
			if(strcmp(path, path2) > 0) // need to switch if path, path2 > 0
			{
				switched = 1; // switch occured, will require at least one more pass
				strcpy(folders_t->path, path2); // save next nodes path to current node
				strcpy(folders_t->next->path, path); // save current nodes path to next node
				folders_t->is_folder = is_folder2; // swap is_folder states
				folders_t->next->is_folder = is_folder1; // swap is_folder states
			}
			folders_t = folders_t->next; // move to next node
		}
	}
}

/* Function: append_folder_list
 *
 * Purpose: add new node to struct and place supplied information to it
 *
 * @params: @folders_ptr -- pointer to struct
            @path -- path for file/folder
            @is_folder -- 0 = file, 1 = folder
 *
 * Return: @none
 *
 * Note: 
 *
*/
int append_folder_list(struct folders * folders_ptr, char path[1000], unsigned char d_type)
{
	int is_folder = 0;

	if(d_type == DT_DIR) // test if supplied path is a directory or not
		is_folder = 1;

	struct folders * conductor; // to traverse linked list
	conductor = folders_ptr; // point to beginning of linked list

	while(conductor->next != NULL) // iterate through until we get to last node
	{
		conductor = conductor->next; // point to next node
	}
	// should be at end of list
	conductor->next = malloc(sizeof(struct folders)); // allocate memory for new list
	conductor = conductor->next; // point to new node
	strncpy(conductor->path, path, sizeof(conductor->path)); // set path for new node
	conductor->is_folder = is_folder; // set folder bit for new node
	conductor->next = NULL; // set next to NULL to indicate this is the last node
}


/* Function: populate_directory
 *
 * Purpose: populate folders linked list with contents of supplied @directory
 *
 * @params: @folders_ptr - pointer to beginning of linked list
 			@directory - string indicating directory to traverse
 *
 * Return: @none
 *
 * Note: 
 *
*/
int populate_directory(struct folders * folders_ptr, char * directory)
{
	struct dirent * dent;
	DIR *src = opendir(directory);
	while((dent = readdir(src)) != NULL)
	{
		if(strcmp("..", dent->d_name) == 0 || strcmp(".", dent->d_name) == 0)
			continue; // skip '.' and '..'

		char path[1000];
		// generate absolute path
		strcpy(path, directory);
		strcat(path, "/");
		strcat(path, dent->d_name);

		append_folder_list(folders_ptr, path, dent->d_type); // add node to struct for newly read opendir

		if(dent->d_type == DT_DIR){ // IF IS A FOLDER
			populate_directory(folders_ptr, path); // recursive traverse
		}

	}

	return 0;
}

/* Function: maketime()
 *
 * Purpose: return datetime string in the format YYYY-M-D H:M:S
 *
 * @params: @none
 *
 * Return: datetime
 *
 * Note: 
 *
*/
char * maketime()
{
	char * datetime = malloc(500);
	char buf[80];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	snprintf(buf, sizeof(buf), "%d-", tm.tm_year + 1900); // YYYY-
	strcpy(datetime, buf);
	snprintf(buf, sizeof(buf), "%d-", tm.tm_mon + 1); // M-
	strcat(datetime, buf);
	snprintf(buf, sizeof(buf), "%d ", tm.tm_mday); // D 
	strcat(datetime, buf);
	snprintf(buf, sizeof(buf), "%d:", tm.tm_hour); //HH:
	strcat(datetime, buf);
	snprintf(buf, sizeof(buf), "%d:", tm.tm_min); // M:
	strcat(datetime, buf);
	snprintf(buf, sizeof(buf), "%d", tm.tm_sec); // S
	strcat(datetime, buf);

	return datetime;
}

/* Function: format_output_file
 *
 * Purpose: prepare file name for output file
 *
 * @params: @none
 *
 * Return: complete path with output file
 *
 * Note: 
 *
*/
char * format_output_file()
{
	char * datetime = maketime(); // must be free'd()
	char file_buf[1000];
	strcpy(file_buf, out_root); // root for output directory
	strcat(file_buf, out_file); // output file name
	strcat(file_buf, "_"); 		// _
	strcat(file_buf, datetime); // datetime
	strcat(file_buf, ".txt");   // extension (.txt)

	free(datetime); // free memory allocated from maketime()
	char * return_path = malloc(strlen(file_buf) + 1); // allocate memory for char* to return

	strcpy(return_path, file_buf); // copy file_buf into return_path

	return return_path; // return return_path
}

/* Function: fix_dir_root
 *
 * Purpose: If dir_root has '/' as it's last character, that needs to be stripped
 *
 * @params: @none
 *
 * Return: none
 *
 * Note: 
 *
*/
void fix_dir_root()
{
	if(dir_root[(strlen(dir_root) - 1)] == '/') // last character in string is a '/'
	{
		dir_root[(strlen(dir_root) -1)] = '\0'; // set '/' to \0
	}
}

/* Function: fix_out_root
 *
 * Purpose: If out_root does not have a '/' as it's last character, that needs to be added
 *
 * @params: @none
 *
 * Return: none
 *
 * Note: 
 *
*/
void fix_out_root()
{
	if(out_root[(strlen(out_root) - 1)] != '/') // last character in string is a '/'
	{
		out_root[(strlen(out_root))] = '/'; // set '/' to \0
		out_root[(strlen(out_root)+1)] = '\0'; // set NULL
	}
}

void display_readme()
{
	FILE *file;
	file = fopen("README", "r"); // open readme for reading
	if(file == NULL) // verify we opened it 
	{
		printf("failed to open readme\n");
		exit(0);
	}

	char line[512]; // buffer for each line
	while(fgets(line, sizeof(line), file) != NULL) // scan line by line, amax 512 char's
	{
		printf("%s", line); // print line
	}
	printf("\n\n"); // print newlines

	fclose(file); // close file
}

int main(int argc, char *argv[])
{
	if(argc == 2)
	{
		if(strcmp(argv[1], "--help") == 0)
		{
			display_readme(); // display readme to screen
			exit(1); // exit
		}
	}

	if(argc == 4) { // 4 arguments. executable | input location | output location | file name
		strncpy(dir_root, argv[1], sizeof(dir_root) - 1);
		strncpy(out_root, argv[2], sizeof(out_root) - 1);
		strncpy(out_file, argv[3], sizeof(out_file) - 1);
	} else {
		strncpy(dir_root, DEFAULT_DIR_ROOT, sizeof(dir_root) - 1);
		strncpy(out_root, DEFAULT_OUT_ROOT, sizeof(out_root) - 1);
		strncpy(out_file, DEFAULT_OUT_FILE, sizeof(out_file) - 1);
	}

	fix_dir_root(); // strip trailing '/' if exists in dir_root
	fix_out_root(); // add trailing '/' if it doesn't exist to out_root

	DIR *src = opendir(out_root); // attempt to open out_root folder location
	if(src == NULL)
	{
		printf("Error: output file location directory doesn't exist or can't be read! -- %s\n", out_root);
		exit(0);
	}
	printf("Output Folder Location valid...\n");

	src = opendir(dir_root); // attempt to open dir_root folder location
	if(src == NULL)
	{
		printf("Error: directory to traverse location doesn't exist or can't be read! -- %s\n", dir_root);
		exit(0);
	}
	printf("Folder to Scan valid...\n");

	struct folders *folders_ptr = (struct folders*)malloc(sizeof(struct folders)); // allocate memory for structure
	folders_ptr->next = NULL; // ->next = NULL, this is the last node in teh list
	
	printf("Populating directory tree...\n");
	populate_directory(folders_ptr, dir_root); // scan directory and save all paths within into struct

	printf("Sorting Directory Tree Alphabetically...\n");
	sort_list(folders_ptr); // bubble sort the list

	printf("Saving directory tree to output file...\n");
	print_to_file(folders_ptr);

	printf("Scan Complete.\n You can find your output file located at: %s\n", out_root);

	return 0;
}
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

int print_to_file(struct folders *folders_ptr, FILE *fp)
{
	struct folders *conductor;
	conductor = folders_ptr;
	while(conductor->next != NULL) // will take us to second to last node
	{
		fprintf(fp, "%s\n", conductor->path);
		conductor = conductor->next;
	}
	fprintf(fp, "%s\n", conductor->path); // last node
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
	//printf("Folder: %s\n", folders_t->path); // last node
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
	int num_passes = 0;
	while(switched == 1) // keep going until an entire pass occurs without a swap
	{
		switched = 0; // reset state
		folders_t = folders_ptr; // point to beginning of list
		while(folders_t->next != NULL) // iterate to the n-1 node
		{
			char path[1000];
			char path2[1000];
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
int append_folder_list(struct folders * folders_ptr, char path[1000], int is_folder)
{
	struct folders * conductor;
	conductor = folders_ptr;
	int counter = 0;
	while(conductor->next != NULL)
	{
		counter++;
		conductor = conductor->next;
	}
	// should be at end of list
	conductor->next = malloc(sizeof(struct folders));
	conductor = conductor->next;
	strncpy(conductor->path, path, sizeof(conductor->path));
	conductor->is_folder = is_folder;
	conductor->next = NULL;
}

int populateDirectory(struct folders * folders_ptr, char * directory)
{
	struct dirent * dent;
	DIR *src = opendir(directory);
	int is_folder = 0;
	while((dent = readdir(src)) != NULL)
	{
		if(strcmp("..", dent->d_name) == 0 || strcmp(".", dent->d_name) == 0)
			continue;

		char path[1000];
		strcpy(path, directory);
		strcat(path, "/");
		strcat(path, dent->d_name);
		is_folder = 0;

		if(dent->d_type == 4){ // FOLDER
			is_folder = 1;
		}

		append_folder_list(folders_ptr, path, is_folder);

		if(dent->d_type == 4){ // FOLDER
			populateDirectory(folders_ptr, path); // recursive traverse
		}

	}
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
	snprintf(buf, sizeof(buf), "%d-", tm.tm_year + 1900);
	strcpy(datetime, buf);
	snprintf(buf, sizeof(buf), "%d-", tm.tm_mon + 1);
	strcat(datetime, buf);
	snprintf(buf, sizeof(buf), "%d ", tm.tm_mday);
	strcat(datetime, buf);
	snprintf(buf, sizeof(buf), "%d:", tm.tm_hour);
	strcat(datetime, buf);
	snprintf(buf, sizeof(buf), "%d:", tm.tm_min);
	strcat(datetime, buf);
	snprintf(buf, sizeof(buf), "%d", tm.tm_sec);
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
	strcpy(file_buf, out_root);
	strcat(file_buf, out_file);
	strcat(file_buf, "_");
	strcat(file_buf, datetime);
	strcat(file_buf, ".txt");
	//printf("file_buf: %s\n", file_buf);
	free(datetime);
	char * return_path = malloc(strlen(file_buf) + 1);
	//printf("%d -- %s\n", strlen(file_buf), file_buf);
	strcpy(return_path, file_buf);
	return return_path;
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
	file = fopen("README", "r");
	if(file == NULL)
	{
		printf("failed to open readme\n");
		exit(0);
	}

	char line[512]; // buffer for each line
	while(fgets(line, sizeof(line), file) != NULL)
	{
		printf("%s", line);
	}
	printf("\n\n");

	fclose(file);
}

int main(int argc, char *argv[])
{
	if(argc == 2)
	{
		if(strcmp(argv[1], "--help") == 0)
		{
			printf("Help:\n\n");
			display_readme();
			exit(1);
		}
	}

	if(argc == 4) // 4 arguments. executable | input location | output location | file name
	{
		printf("command line set\n");
		strncpy(dir_root, argv[1], sizeof(dir_root) - 1);
		strncpy(out_root, argv[2], sizeof(out_root) - 1);
		strncpy(out_file, argv[3], sizeof(out_file) - 1);
	} else {
		strncpy(dir_root, DEFAULT_DIR_ROOT, sizeof(dir_root) - 1);
		strncpy(out_root, DEFAULT_OUT_ROOT, sizeof(out_root) - 1);
		strncpy(out_file, DEFAULT_OUT_FILE, sizeof(out_file) - 1);
	}
	fix_dir_root(); // strip trailing '/' if exists in dir_root
	fix_out_root(); // add trailing '/' if it doesn't exist

	DIR *src = opendir(out_root);
	if(src == NULL)
	{
		printf("out_root directory unable to be opened: %s\n", out_root);
		exit(0);
	}

	printf("dir_root: %s\n out_root: %s\n out_file: %s\n", dir_root, out_root, out_file);

	struct folders *folders_ptr = (struct folders*)malloc(sizeof(struct folders));
	folders_ptr->next = NULL;
	
	populateDirectory(folders_ptr, dir_root); // scan directory and save all paths within into struct

	sort_list(folders_ptr); // bubble sort the list

	FILE *file; 

	char * output_file = format_output_file();
	printf("output file: %s\n", output_file);
	
	file = fopen(output_file,"w");

	free(output_file);

	print_to_file(folders_ptr, file);

	fclose(file);

	printf("done scanning\n");

	return 0;
}
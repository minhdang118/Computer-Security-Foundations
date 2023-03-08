#define _GNU_SOURCE
#define _POSIX_SOURCE

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <sys/syscall.h>

#include "runas.h"

int main(int argc, char const *argv[])
{
    struct termios term;
	struct passwd *p;
	
    char * username;
    char * program;
	char * arguments[argc-2];
    char password[32];
	int data_fd;
	int data_length;
	int log_fd;
	int exit_code;
	unsigned char data_buffer[1024];
	uid_t euid;

	char * data_dir = "/etc/runas";
	char * log_dir = "/tmp/runaslog";
    
    // Initialize username, program and arguments
    username = argv[1];
    program = argv[2];
	for (int i = 3; i < argc; i++)
	{
		arguments[i-3] = argv[i];
	}
	arguments[argc-3] = (char*)0;

	// Set GIDs and UIDs
	if ((setresgid (101, 101, 101)) == -1) 
	{
		perror ("Failed to set group IDs");
		return 1;
	}

	if ((setresuid (federer, federer, federer)) == -1) 
	{
		perror ("Failed to set user IDs");
		return 1;
	}
    
    // Prompt for a password
    printf("%s", "Enter password:");

    tcgetattr(fileno(stdin), &term);
    term.c_lflag &= ~ECHO;

    tcsetattr(fileno(stdin), 0, &term);
    fgets(password, sizeof(password), stdin);
	password[strcspn(password, "\n")] = '\0';
    
    term.c_lflag |= ECHO;
    tcsetattr(fileno(stdin), 0, &term);

	printf("%s\n", "\n");

	// Open and read database
	if ((data_fd = open (data_dir, O_RDONLY)) == -1) 
	{
		perror ("Failed to open database");
		return 1;
	} 
	else 
	{
		if ((data_length = read (data_fd, data_buffer, sizeof (data_buffer))) == -1) 
		{
			perror ("Failed to read database");
			return 1;
		}

		euid = geteuid();
		
		if ((p = getpwuid(euid)) == NULL)
		{
			perror("Failed to get user information");
			return 1;
		}
		else
		{
			// Set up for login information checking
			char * data_line = strtok(data_buffer, "\n");
			char data_to_check[128];

			char id_name[32];
			strncpy(id_name, p->pw_name, sizeof (id_name) - 1);
			id_name[31] = '\0';

			char login_name[32];
			strncpy(login_name, username, sizeof (login_name) - 1);
			login_name[31] = '\0';

			char colon[2] = ":";
			strcat(data_to_check, id_name);
			strcat(data_to_check, colon);
			strcat(data_to_check, login_name);
			strcat(data_to_check, colon);
			strcat(data_to_check, password);

			// Check login information
			while (data_line != NULL)
			{
				if (strcmp(data_to_check, data_line) == 0)
				{
					// RUN PROGRAM
					char *newenviron[] = {NULL};
					// printf("%d\n", sizeof(arguments));
					// for (int i = 0; i < argc-2; i++)
					// {
					// 	printf("%s\n", arguments[i]);
					// }
					
					exit_code = execve(program, arguments, newenviron);
					printf("%d\n", exit_code);
					if (exit_code == -1) 
					{
						perror ("Failed to execute program");
						return 1;
					}

					// Write to log file
					printf("%s\n", "Got here 1");
					if ((log_fd = open (log_dir, O_WRONLY)) == -1) 
					{
						perror ("Failed to open log file");
						return 1;
					} 

					printf("%s\n", "Got here 2");

					if ((write (log_fd, program, strlen (program))) == -1)
					{
						perror ("Failed to write program to log file");
						return 1;
					}

					if ((write (log_fd, " ", 2)) == -1)
					{
						perror ("Failed to write space to log file");
						return 1;
					}

					for (size_t i = 0; i < sizeof(arguments); i++)
					{
						if ((write (log_fd, arguments[i], strlen (arguments[i]))) == -1)
						{
							perror ("Failed to write argument to log file");
							return 1;
						}
						if ((write (log_fd, " ", 2)) == -1)
						{
							perror ("Failed to write space to log file");
							return 1;
						}
					}

					char str_code[4];
					sprintf(str_code, "%d", exit_code);

					if ((write (log_fd, str_code, strlen (str_code))) == -1)
					{
						perror ("Failed to write exit code to log file");
						return 1;
					}
					
					if ((write (log_fd, "\n", 2)) == -1)
					{
						perror ("Failed to write new line to log file");
						return 1;
					}
					
					return 0;
				}
				data_line = strtok(NULL, "\n");
			}
			printf("%s\n", "Wrong password and/or username!");
		}
	}
    return 0;
}

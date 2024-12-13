/*
 ============================================================================
 Name        : CLIENT.c
 Author      : Gravili Sebastiano
 Version     :
 Copyright   : Gravili Sebastiano
 Description : A UDP client program for generating passwords based on user-specified criteria, communicating with a remote server to request and retrieve the generated passwords.
 ============================================================================
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include "protocol.h"

int delay(int seconds);
int void_control(char *str);
int format_control(char *str);
int digitLimit_control(char *str);
void show_help_menu();

void clearwinsock() {
	#if defined WIN32
		WSACleanup();
	#endif
}

void errorhandler(char *errorMessage) {
	printf("%s", errorMessage);
}

int main(void) {
		#if defined WIN32
			// Initialize Winsock
			WSADATA wsa_data;
			int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
			if (result != NO_ERROR) {
				printf("Error at WSAStartup()\n");
				return 0;
			}
		#endif

	// create client socket
		int c_socket;
		c_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (c_socket < 0) {
			errorhandler("socket creation failed.\n");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}

		const char* name = "passwdgen.uniba.it";
		struct hostent* host;
		struct in_addr* ina;
		host = gethostbyname(name);
		if(host == NULL){
			fprintf(stderr, "gethostbyname() failed.\n");
			exit(EXIT_FAILURE);
		}
		else{
			ina = (struct in_addr*) host->h_addr_list[0];
		}

	// set connection settings
		struct sockaddr_in echoClient;
		memset(&echoClient, 0, sizeof(echoClient));
		echoClient.sin_family = PF_INET;
		echoClient.sin_addr.s_addr = inet_addr(inet_ntoa(*ina)); // IP del server
		echoClient.sin_port = htons(PROTO_PORT); // Server port

		// get data from user

		int esc;
		printf("CLIENT PORT:%d\n\nEnter the type of password to generate followed by the desired length (e.g., n 8 = for a numeric password of 8 characters):\n"
				"Enter h to display the help menu.\n", echoClient.sin_port);
		// Do-While loop to continue data input, sending, and receiving between the client and the server
		// * until the termination character 'q' is entered
		do{
			msg_code m;
			struct sockaddr_in fromAddr;
			unsigned int fromSize;
			int echo_String_leng;
			int resp_String_leng;
			esc = 0;
			char *string = (char *)calloc(ECHOMAX, sizeof(char));
			printf("\n=\t");
			echo_String_leng = strlen(fgets(string, ECHOMAX, stdin));
			if(echo_String_leng <= ECHOMAX){
				string[strlen(string) - 1] = '\0';
				if (void_control(string)){
					int format = 0;
					format = format_control(string);
					if(format == 1){
						memset(&m, '\0', sizeof(msg_code));
						strcpy(m.type, strtok(string, " "));
						strcpy(m.number, strtok(NULL, "\0"));
						if(digitLimit_control(m.number)){
							// send data to server
							if (sendto(c_socket, &m, echo_String_leng, 0, (SOCKADDR*)&echoClient, sizeof(echoClient)) != echo_String_leng){
								errorhandler("send() sent a different number of bytes than expected");
								closesocket(c_socket);
								return -1;
							}

							char *password = (char *)calloc(atoi(m.number), sizeof(char *));

							//receive string echo from server
							fromSize = sizeof(fromAddr);
							resp_String_leng = recvfrom(c_socket, password, ECHOMAX, 0, (SOCKADDR*)&fromAddr, &fromSize);
							if (echoClient.sin_addr.s_addr != fromAddr.sin_addr.s_addr){
								fprintf(stderr, "Error: received a packet from unknown source.\n");
								closesocket(c_socket);
								exit(EXIT_FAILURE);
							}
							printf("Generated Password: %s\n", password);
							free(password);
							/*At this point in the program, there is a delay.
							 * This controls the speed at which strings are entered into the program,
							 * ensuring its correct operation.*/
							delay(3);
						}
						else
							errorhandler("INSERT A VALID STRING:: password too short/long, insert a number between 6 and 32\n");
					}
					else if(format == 2){ //The format 2 is used when you want close the client
						esc = 1;
						memset(&m, '\0', sizeof(msg_code));
						strcpy(m.type, string);
						// send data to server
						if (sendto(c_socket, &m, echo_String_leng, 0, (SOCKADDR*)&echoClient, sizeof(echoClient)) != echo_String_leng){
							errorhandler("send() sent a different number of bytes than expected");
							closesocket(c_socket);
							return -1;
						}
						memset(string, '\0', strlen(string));
						//receive string echo from server
						fromSize = sizeof(fromAddr);
						resp_String_leng = recvfrom(c_socket, string, ECHOMAX, 0, (SOCKADDR*)&fromAddr, &fromSize);
						if (echoClient.sin_addr.s_addr != fromAddr.sin_addr.s_addr){
							fprintf(stderr, "Error: received a packet from unknown source.\n");
							closesocket(c_socket);
							exit(EXIT_FAILURE);
						}
						printf("%s\n\n", string);
						#if defined WIN32
							system("pause");
						#endif
						return 0;
					}
					else if(format == 3)
						errorhandler("INSERT A VALID STRING:: password type doesn't match\n");
					else if(format == 4)
						show_help_menu();
					else if(format == 0)
						errorhandler("INSERT A VALID STRING:: string doesn't match the format, insert a string with this format(n 10)\n");
				}
				else{
					errorhandler("INSERT A VALID STRING:: string is empty\n");
				}
					free(string);
			}
			else
				errorhandler("INSERT A VALID STRING:: string is too long\n");
		}
		while(esc == 0); // end Do-While

		closesocket(c_socket);
		clearwinsock();
		system("pause");

		#if defined WIN32
			system("pause");
		#endif

		return 0;
} // main end

int delay(int seconds) {
	time_t start_time, finish_time, current_time;

	time(&start_time);
	finish_time = start_time + seconds;
	while(time(&current_time) < finish_time){}
	return 1;
}

/* Function used to check if a valid and available password type has been entered
 * If the type is valid, it returns 1;
 * If the type is 'q', it returns 2;
 * In any other case, it returns 3. */
int method_control(char method){
	if (method == 'n' || method == 'a' || method == 'm' || method == 's' || method == 'u')
		return 1;
	else if(method == 'q')
		return 2;
	else if(method == 'h')
		return 4;
	else
		return 3;
}

/* Function used to check if the entered string is empty or not
 * If it is NOT empty, it returns 1;
 * If it is empty, it returns 0. */
int void_control(char *str){
	if (strcmp(str, "\0") == 0)
		return 0;
	else
		return 1;
}

/* Function used to check if the string complies with the predefined input format ('type''space''number')
 * If the first character of the string is NOT alphabetical or is recognized as an invalid type, it returns 3;
 * If the first character of the string is alphabetical but is recognized as the termination character 'q', it returns 2;
 * If the first character of the string is alphabetical and is recognized as a valid type, the next check is performed;
 * If the second character of the string is NOT a space, it returns 0;
 * If the second character of the string is a space, it checks whether all characters following the space are numbers.
 * If there are non-numeric characters, it returns 0;
 * If all checks are passed, the function returns 1. */
int format_control(char *str){
	int method = 0;
	if(isalpha(str[0]) == 0 || (method = method_control(str[0])) != 1){
		return method;
	}
	else if(str[1] != ' ')
		return 0;
	else{
		for(int i = 2; i < strlen(str); i++){
			if(isdigit(str[i]) == 0)
				return 0;
		}
	}
	return 1;
}

/* Function used to check if the length of the password to be created is between 6 and 32, inclusive
 * If the number satisfies this condition, it returns 1;
 * If the number does NOT satisfy this condition, it returns 0. */
int digitLimit_control(char *str){
	int number = atoi(str);
	if(number >= 6 && number <= 32)
		return 1;
	else
		return 0;
}

/* Function used to show the help menu*/
void show_help_menu(){
	printf(" Password Generator Help Menu\n"
			" Commands:\n"
			" h        : show this help menu\n"
			" n LENGTH : generate numeric password (digits only)\n"
			" a LENGTH : generate alphabetic password (lowercase letters)\n"
			" m LENGTH : generate mixed password (lowercase letters and numbers)\n"
			" s LENGTH : generate secure password (uppercase, lowercase, numbers, symbols)\n"
			" u LENGTH : generate unambiguous secure password (no similar-looking characters)\n"
			" q        : quit application\n\n"
			" LENGTH must be between 6 and 32 characters\n\n"
			" Ambiguous characters excluded in 'u' option:\n"
			" 0 O o (zero and letters O)\n"
			" 1 l I i (one and letters l, I)\n"
			" 2 Z z (two and letter Z)\n"
			" 5 S s (five and letter S)\n"
			" 8 B (eight and letter B)\n");
}

/*
 ============================================================================
 Name        : SERVER.c
 Author      : Gravili Sebastiano
 Version     :
 Copyright   : Gravili Sebastiano
 Description : A UDP server program that generates passwords based on client requests.
 	 	 	   Offering options for numeric, alphabetic, mixed, secure, and unambiguous passwords, and communicates responses back to the client.
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
#include <stdlib.h>
#include <time.h>
#include "protocol.h"

char* generate_numeric(size_t lenght);
char* generate_alpha(size_t lenght);
char* generate_mixed(size_t lenght);
char* generate_sicure(size_t lenght);
char* generate_unambiguous(size_t lenght);

void errorhandler(char *errorMessage){
	printf("%s", errorMessage);
}

void clearwinsock() {
	#if defined WIN32
		WSACleanup();
	#endif
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

	//create welcome socket
		int my_socket;
		my_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(my_socket < 0){
			errorhandler("socket creation failed.\n");
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

	//set connection setting
		struct sockaddr_in echoServ;
		memset(&echoServ, 0, sizeof(echoServ));
		echoServ.sin_family = AF_INET;
		echoServ.sin_addr.s_addr = inet_addr(inet_ntoa(*ina));
		echoServ.sin_port = htons(PROTO_PORT);
		if(bind(my_socket, (SOCKADDR*) &echoServ, sizeof(echoServ)) < 0){
			errorhandler("bind() failed.\n");
			closesocket(my_socket);
			clearwinsock();
			return -1;
		}

	//receive string for client
		unsigned int client_Addr_Length;
		struct sockaddr_in echoClien;
		msg m;
		int recvMsg_size;
		// get msg from client
		do{
			client_Addr_Length = sizeof(echoClien);
			if ((recvMsg_size = recvfrom(my_socket, &m, ECHOMAX, 0, (SOCKADDR*)&echoClien, &client_Addr_Length)) == -1) {
				errorhandler("\nConnection closed:: Data reception failed or connection closed prematurely\n\n");
				break;
			}
			char* result = (char *) calloc(atoi(m.number), sizeof(char *));
			if (m.type[0] != 'q'){
				printf("New request from %s:%d\n", inet_ntoa(echoClien.sin_addr), echoClien.sin_port);
				if (m.type[0] == 'n')
					result = generate_numeric(atoi(m.number));
				else if (m.type[0] == 'a')
					result = generate_alpha(atoi(m.number));
				else if (m.type[0] == 'm')
					result = generate_mixed(atoi(m.number));
				else if (m.type[0] == 's')
					result = generate_sicure(atoi(m.number));
				else if (m.type[0] == 'u')
					result = generate_unambiguous(atoi(m.number));

				//send string echo to client
				if (sendto(my_socket, result, *m.number, 0, (struct sockaddr *)&echoClien, sizeof(echoClien)) != *m.number)
					errorhandler("sendto() sent different number of bytes than expected");

				printf("\tPassword generation successful\n\n");
				free(result);
			}
			else{
				// send "Connection Closed..." to client
				result = "Connection Closed...";
				if (sendto(my_socket, result, strlen(result), 0, (struct sockaddr *)&echoClien, sizeof(echoClien)) != strlen(result)){
					errorhandler("sendto() sent different number of bytes than expected");
					closesocket(my_socket);
					clearwinsock();
					break;
				}

				printf("Connection from %s:%d CLOSED\n\n", inet_ntoa(echoClien.sin_addr), echoClien.sin_port);
				break;
			}
		}
		while(1);

		closesocket(my_socket);
		clearwinsock();

		return 0;
} // main end

/* Function that counts the occurrences of each character before it is added to the final array.
 * This ensures the creation of passwords with the minimum number of occurrences for each generated character.
 * It returns the number of occurrences present in the array, including the one passed as a reference,
 * as it may already be part of the array. */
int linear_search_char(char vector[], char let){
	int count = 1;
	for (int i = 0; i < strlen(vector); i++){
		if (vector[i] == let){
			count++;
		}
	}
	return count;
}

/* Function that ensures no ambiguous character has a similar one in the array before it is added to the final array.
 * Returns 1 if there are no previous occurrences of similar ambiguous character;
 * Returns 0 if there are previous occurrences.
 * First, it checks whether the character to be added is present in the `ambiguous_letter` array:
 * If it is present, the entire array is scanned to verify if any index contains a character similar to the one to be added;
 * If a similar character is found, the function terminates. */
int linear_search_unambiguous(char vector[], char let){
	if (strchr("0Oo1lIi2Zz5Ss8B", let) != NULL) {
		for (int i = 0; i < strlen(vector); i++) {
			switch(let) {
				case '0' :
					if (vector[i] == 'O' || vector[i] == 'o')
						return 0;
				break;
				case 'O' :
					if (vector[i] == '0' || vector[i] == 'o')
						return 0;
				break;
				case 'o' :
					if (vector[i] == '0' || vector[i] == 'O')
						return 0;
				break;
				case '1' :
					if (vector[i] == 'l' || vector[i] == 'I' || vector[i] == 'i')
						return 0;
				break;
				case 'l' :
					if (vector[i] == '1' || vector[i] == 'I' || vector[i] == 'i')
						return 0;
				break;
				case 'I' :
					if (vector[i] == '1' || vector[i] == 'l' || vector[i] == 'i')
						return 0;
				break;
				case 'i' :
					if (vector[i] == '1' || vector[i] == 'l' || vector[i] == 'I')
						return 0;
				break;
				case '2' :
					if ( vector[i] == 'Z' || vector[i] == 'z')
						return 0;
				break;
				case 'Z' :
					if (vector[i] == '2' ||vector[i] == 'z')
						return 0;
				break;
				case 'z' :
					if (vector[i] == '2' || vector[i] == 'Z')
						return 0;
				break;
				case '5' :
					if (vector[i] == 'S' || vector[i] == 's')
						return 0;
				break;
				case 'S' :
					if (vector[i] == '5' ||vector[i] == 's')
						return 0;
				break;
				case 's' :
					if (vector[i] == '5' || vector[i] == 'S')
						return 0;
				break;
				case '8' :
					if (vector[i] == 'B')
						return 0;
				break;
				case 'B' :
					if (vector[i] == '8')
						return 0;
				break;
			}
		}
	}

	return 1;
}

/* Function that randomly generates a number from 0 to 9 and returns it as a character.
 * The generation uses ASCII codes to enable the rand function, which randomizes only integer numbers. */
char random_digit(){
	return (char)(rand() % 10) + '0';
}

/* Function that randomly generates a lowercase letter from 'a' to 'z' and returns it as a character.
 * The generation uses ASCII codes to enable the rand function, which randomizes only integer numbers. */
char random_lowercase(){
	return (char)(rand() % 26) + 'a';
}

/* Function that randomly generates an uppercase letter from A to Z and returns it as a character.
 * The generation uses the ASCII code to enable the random function, which operates only on integers. */
char random_uppercase(){
	return (char)(rand() % 26) + 'A';
}

/* Function that randomly generates a symbol from those available in an internal array
 * and returns it as a character. The generation uses ASCII codes to enable the randomization process,
 * as rand can only randomize integers. */
char random_symbol(){
	char symbol[] = {'!', '"', '#', '$', '%', '&', '(', ')', '*', '+', '-', '/', ':', ';', '<', '=', '>', '?', '@', '[', ']', '^', '_', '{', '|', '}'};
	return symbol[(rand() % strlen(symbol))];
}

/* Function that generates and returns a mixed password by randomly selecting either a number.
 * Each step generates a different character, stores it in R_temp, which is then saved into an array.
 * At the end of the function, this array is copied into a pointer, which is returned outside the function. */
char* generate_numeric(size_t lenght){
	srand(time(NULL));
	char temp[lenght];
	char R_temp;
	char* ris = (char *) calloc(lenght, sizeof(char *));
	for (int i = 0; i < lenght; i++){
		/*Numbers, generated randomly and converted into characters, are added to a password only if no digit occurs more than twice;
		 * otherwise, the generation is repeated. This rule applies up to a maximum of 20 numbers: a string with digits 0–9 repeated twice each reaches 20 characters.
		 * Beyond this limit, adding another number would create an infinite loop, as a third occurrence of one of the digits would become unavoidable.*/
		if (lenght < 21 && lenght > 0){
			if (i == 0) {
				R_temp = random_digit();
				temp[i] = R_temp;
			}
			else{
				do{
					R_temp = random_digit();
				}
				while(linear_search_char(temp, R_temp) > 2);

				temp[i] = R_temp;
			}
		}
		else{
			R_temp = random_digit();
			temp[i] = R_temp;
		}
	}
	strncpy(ris, temp, lenght);

	return ris;
}

/* Function that generates and returns a mixed password by randomly selecting either a lowercase letter.
 * Each step generates a different character, stores it in R_temp, which is then saved into an array.
 * At the end of the function, this array is copied into a pointer, which is returned outside the function. */
char* generate_alpha(size_t lenght){
	srand(time(NULL));
	char temp[lenght];
	char R_temp;
	char* ris = (char *) calloc(lenght, sizeof(char *));
	for (int i = 0; i < lenght; i++){
		/*Numbers, generated randomly and converted into characters, are added to a password only if no digit occurs more than twice;
		 * otherwise, the generation is repeated. This rule applies up to a maximum of 20 numbers: a string with digits 0–9 repeated twice each reaches 20 characters.
		 * Beyond this limit, adding another number would create an infinite loop, as a third occurrence of one of the digits would become unavoidable.*/
		if (lenght < 21 && lenght > 0){
			if (i == 0) {
				R_temp = random_lowercase();
				temp[i] = R_temp;
			}
			else{
				do{
					R_temp = random_lowercase();
				}
				while(linear_search_char(temp, R_temp) > 2);

				temp[i] = R_temp;
			}
		}
		else{
			R_temp = random_lowercase();
			temp[i] = R_temp;
		}
	}
	strncpy(ris, temp, lenght);

	return ris;
}

/* Function that generates and returns a mixed password by randomly selecting either a number
 * or a lowercase letter.
 * Each step generates a different character, stores it in R_temp, which is then saved into an array.
 * At the end of the function, this array is copied into a pointer, which is returned outside the function. */
char* generate_mixed(size_t lenght){
	srand(time(NULL));
	char temp[lenght];
	char R_temp;
	char* ris = (char *) calloc(lenght, sizeof(char *));
	for (int i = 0; i < lenght; i++){
		int version = (rand() % 2);
		/*Numbers, generated randomly and converted into characters, are added to a password only if no digit occurs more than twice;
		 * otherwise, the generation is repeated. This rule applies up to a maximum of 20 numbers: a string with digits 0–9 repeated twice each reaches 20 characters.
		 * Beyond this limit, adding another number would create an infinite loop, as a third occurrence of one of the digits would become unavoidable.*/
		if (lenght < 21 && lenght > 0){
			if (i == 0) {
				if (version == 0) R_temp = random_lowercase();
				else if(version == 1) R_temp = random_digit();
				temp[i] = R_temp;
			}
			else{
				do{
					if (version == 0) R_temp = random_lowercase();
					else if(version == 1) R_temp = random_digit();
				}
				while(linear_search_char(temp, R_temp) > 2);

				temp[i] = R_temp;
			}
		}
		else{
			if (version == 0) R_temp = random_lowercase();
			else if(version == 1) R_temp = random_digit();
			temp[i] = R_temp;
		}
	}
	strncpy(ris, temp, lenght);

	return ris;
}

/* Function that generates and returns a secure password by randomly selecting a number,
 * a lowercase letter, an uppercase letter, or a symbol.
 * Each step generates a different character, stores it in R_temp, which is then saved into an array.
 * At the end of the function, this array is copied into a pointer, which is returned outside the function. */
char* generate_sicure(size_t lenght){
	srand(time(NULL));
	char temp[lenght];
	char R_temp;
	char* ris = (char *) calloc(lenght, sizeof(char *));
	for (int i = 0; i < lenght; i++){
		int version = (rand() % 4);
		/*Numbers, generated randomly and converted into characters, are added to a password only if no digit occurs more than twice;
		 * otherwise, the generation is repeated. This rule applies up to a maximum of 20 numbers: a string with digits 0–9 repeated twice each reaches 20 characters.
		 * Beyond this limit, adding another number would create an infinite loop, as a third occurrence of one of the digits would become unavoidable.*/
		if (lenght < 21 && lenght > 0){
			if (i == 0) {
				if (version == 0) R_temp = random_lowercase();
				else if(version == 1) R_temp = random_digit();
				else if(version == 2) R_temp = random_uppercase();
				else if(version == 3) R_temp = random_symbol();
				temp[i] = R_temp;
			}
			else{
				do{
					if (version == 0) R_temp = random_lowercase();
					else if(version == 1) R_temp = random_digit();
					else if(version == 2) R_temp = random_uppercase();
					else if(version == 3) R_temp = random_symbol();
				}
				while(linear_search_char(temp, R_temp) > 2);

				temp[i] = R_temp;
			}
		}
		else{
			if (version == 0) R_temp = random_lowercase();
			else if(version == 1) R_temp = random_digit();
			else if(version == 2) R_temp = random_uppercase();
			else if(version == 3) R_temp = random_symbol();
			temp[i] = R_temp;
		}
	}
	strncpy(ris, temp, lenght);

	return ris;
}

/* Function that generates and returns a secure password by randomly selecting a number,
 * a lowercase letter, an uppercase letter, or a symbol.
 * The generated password is secure and does not include ambiguous characters, i.e., those that might look visually similar.
 * Each step generates a different character, stores it in R_temp, which is then saved into an array.
 * Additionally, all generated characters are checked to ensure none are ambiguous before being added.
 * At the end of the function, this array is copied into a pointer, which is returned outside the function. */
char* generate_unambiguous(size_t lenght){
	srand(time(NULL));
	char temp[lenght];
	char R_temp;
	char* ris = (char *) calloc(lenght, sizeof(char *));
	for (int i = 0; i < lenght; i++){
		int version = (rand() % 3);
		/*Numbers, generated randomly and converted into characters, are added to a password only if no digit occurs more than twice;
		 * otherwise, the generation is repeated. This rule applies up to a maximum of 20 numbers: a string with digits 0–9 repeated twice each reaches 20 characters.
		 * Beyond this limit, adding another number would create an infinite loop, as a third occurrence of one of the digits would become unavoidable.*/
		if (lenght < 21 && lenght > 0){
			if (i == 0) {
				if (version == 0) R_temp = random_lowercase();
				else if(version == 1) R_temp = random_digit();
				else if(version == 2) R_temp = random_uppercase();
				else if(version == 3) R_temp = random_symbol();
				temp[i] = R_temp;
			}
			else{
				do{
					if (version == 0) R_temp = random_lowercase();
					else if(version == 1) R_temp = random_digit();
					else if(version == 2) R_temp = random_uppercase();
					else if(version == 3) R_temp = random_symbol();
				}
				while(linear_search_char(temp, R_temp) > 2 || !linear_search_unambiguous(temp, R_temp));

				temp[i] = R_temp;
			}
		}
		else{
			do{
				if (version == 0) R_temp = random_lowercase();
				else if(version == 1) R_temp = random_digit();
				else if(version == 2) R_temp = random_uppercase();
				else if(version == 3) R_temp = random_symbol();
			}
			while(!linear_search_unambiguous(temp, R_temp));
			temp[i] = R_temp;
		}
	}
	strncpy(ris, temp, lenght);

	return ris;
}




/*
 * protocol.h
 *
 *  Created on: 12 dic 2024
 *      Author: Gravili Sebastiano
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define PROTO_PORT 60000
#define ECHOMAX 255

typedef struct{
	char type[1];
	char number[ECHOMAX];
}msg;

#endif /* PROTOCOL_H_ */

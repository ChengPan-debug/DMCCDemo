/**
 * \file usefulStruct.h
 */

#ifndef USEFULSTRUCT_H
#define USEFULSTRUCT_H

typedef struct
{
	unsigned length;
	unsigned char *buf;
	bool isEmtpy() {
		return length == 0;
	}
	void Trim() {
		if (this->isEmtpy()) {
			return;
		}
		while (length && !buf[length - 1]) 
			--length;
		
	}
}PacketNode_t;

#endif //USEFULSTRUCT_H
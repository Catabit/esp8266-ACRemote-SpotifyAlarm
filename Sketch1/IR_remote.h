#pragma once
#include <ir_Coolix.h>
#include "AC_commands.h"

#define IR_LED 14


class IR_remote
{
public:
	IR_remote();
	~IR_remote();

	void sendCommand(int command);

private:
	//IRsend* ac;  // Set the GPIO to be used to sending the message.
	IRCoolixAC* ac;

	//const uint32_t kCoolixDirect = 0b101100100000111111100000;  // 0xB20FE0 TODO: use this if you need it with ac->setRaw(..)

};


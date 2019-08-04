#include "IR_remote.h"

IR_remote::IR_remote()
{
	//irsend = new IRsend(IR_LED);
	ac = new IRCoolixAC(IR_LED);
	ac->begin();

}


IR_remote::~IR_remote()
{
}

void IR_remote::sendCommand(int command) {
	if (command >= 16 && command <= 30) {
		ac->setTemp(command);
	}
	else {
		switch (command) {
		case OFF:
			ac->off();
			break;
		case SWING:
			ac->setSwing();
			break;
		case DIRECT:
			//TODO: use the raw code
			break;
		case SLEEP:
			ac->setSleep();
			break;
		case LED:
			ac->setLed();
			break;
		case TURBO:
			ac->setTurbo();
			break;
		case SILENT:
			// TODO: add raw
			break;
		case FANMIN:
			ac->setFan(kCoolixFanMin, true);
			break;
		case FANMED:
			ac->setFan(kCoolixFanMed, true);
			break;
		case FANMAX:
			ac->setFan(kCoolixFanMax, true);
			break;
		case FANAUTO:
			ac->setFan(kCoolixFanAuto, true);
			break;
		case MAUTO:
			ac->setMode(kCoolixAuto);
			break;
		case MCOOL:
			ac->setMode(kCoolixCool);
			break;
		case MDRY:
			ac->setMode(kCoolixDry);
			break;
		case MFAN:
			ac->setMode(kCoolixFan);
			break;
		case MHEAT:
			ac->setMode(kCoolixHeat);
			break;
		}
	}
	ac->send(1);
}

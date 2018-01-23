// HMM.cpp : 定义控制台应用程序的入口点。
//

#include "../HMM/HMM.h"

char hiddenStateName[][10] = {
	"sun",
	"cloud",
	"rain"
};
float hiddenStateRate[] = {
	1.0,
	0,
	0
};

float hiddenStateTrans[][3] = {
	{
		0.5, 0.375, 0.125
	}, {
		0.25, 0.125, 0.625
	}, {
		0.25, 0.375, 0.375
	}
};

char shownStateName[][10] = {
	"dry",
	"dryish",
	"damp",
	"soggy"
};
float shownStateTrans[][4] = {
	{
		0.6, 0.2, 0.15, 0.05
	}, {
		0.25, 0.25, 0.25, 0.25
	}, {
		0.05, 0.10, 0.35, 0.5
	}
};


int _tmain(int argc, _TCHAR* argv[])
{
	HMM hmm;
	for (int i = 0; i < 3; i++) {
		hmm.hidden.addState(hiddenStateName[i], 1);
	}
	for (int i = 0; i < 2; i++) {
		hmm.hidden.makeRate(hiddenStateName[i], hiddenStateName[i + 1], hiddenStateRate[i]);
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			hmm.hidden.makeTransRate(hiddenStateName[i], hiddenStateName[j],
				hiddenStateTrans[i][j]);
		}
	}
	for (int i = 0; i < 4; i++) {
		hmm.shown.addState(shownStateName[i]);
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++) {
			hmm.makeConfusionRate(hiddenStateName[i], shownStateName[j],
				shownStateTrans[i][j]);
		}
	}


	//TEST
	hmm.printCurrentRate();
	hmm.printTransitionRate();
	hmm.printTransitionBackRate();
	hmm.printObversableStates();
	hmm.printConfusionRate();
	hmm.printConfusionBackRate();

	hmm.hidden.getState("rain")->conf.getPos(2)->weight = 10;

	hmm.printConfusionRate();
	hmm.printConfusionBackRate();


	_getch();

	return 0;
}


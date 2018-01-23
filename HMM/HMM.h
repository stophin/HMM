//this->h
//

#ifndef _HMM_H_
#define _HMM_H_

#include "../common/MultiLinkList.h"

class HMMState;
class Connector {
public:
	Connector(EFTYPE v) :
		back(NULL),
		forw(NULL),
		weight(v){
	}
	~Connector() {
		back = NULL;
		forw = NULL;
	}

	HMMState * back;
	HMMState * forw;

	EFTYPE weight;

	// for multilinklist
	// The number of Connectors will be the max hidden layer numbers + 1
#define Connector_Size 2
	INT uniqueID;
	Connector * prev[Connector_Size];
	Connector * next[Connector_Size];
	void operator delete(void * _ptr){
		if (_ptr == NULL)
		{
			return;
		}
		for (INT i = 0; i < Connector_Size; i++)
		{
			if (((Connector*)_ptr)->prev[i] != NULL || ((Connector*)_ptr)->next[i] != NULL)
			{
				return;
			}
		}
		delete(_ptr);
	}
};
class HMMState {
public:
	HMMState(const char * name, int linkindex = 0) :
		conn(0), tran(0), conf(linkindex), trans_back(1), name(NULL) {
		if (NULL == name) {
			return;
		}
		int len = strlen(name);
		this->name = new char[len + 1];
		memcpy(this->name, name, len);
		this->name[len] = 0;
	}
	~HMMState() {
		if (this->name) {
			delete[] this->name;
			this->name = NULL;
		}
		tran.~MultiLinkList();
	}

	char *name;

	Connector conn;
	MultiLinkList<Connector> tran;
	MultiLinkList<Connector> trans_back;
	MultiLinkList<Connector> conf;
	// for multilinklist
#define HMMState_Size 2
	INT uniqueID;
	HMMState * prev[HMMState_Size];
	HMMState * next[HMMState_Size];
	void operator delete(void * _ptr){
		if (_ptr == NULL)
		{
			return;
		}
		for (INT i = 0; i < HMMState_Size; i++)
		{
			if (((HMMState*)_ptr)->prev[i] != NULL || ((HMMState*)_ptr)->next[i] != NULL)
			{
				return;
			}
		}
		delete(_ptr);
	}

};

class StateContainer : public MultiLinkList<HMMState> {
public:
	StateContainer(int linkindex) : MultiLinkList<HMMState>(linkindex) {
	}

	HMMState * getState(const char * name) {
		HMMState * state = this->link;
		if (state) {
			do {
				if (!strcmp(state->name, name)) {
					return state;
				}

				state = this->next(state);
			} while (state && state != this->link);
		}
		return NULL;
	}
	void addState(const char * name, int linkindex = 0) {
		if (getState(name)) {
			return;
		}
		HMMState * state = new HMMState(name, linkindex);
		if (NULL == state) {
			return;
		}
		this->insertLink(state);
	}
	void makeRate(const char * back, const char * forw,
		float w_back) {
		HMMState * s_back = getState(back);
		HMMState * s_forw = getState(forw);
		if (NULL == s_back || NULL == s_forw) {
			return;
		}
		if (s_back == s_forw) {
			return;
		}
		Connector * conn = &s_back->conn;
		if (NULL == conn) {
			return;
		}
		conn->back = s_back;
		conn->forw = s_forw;
		conn->weight = w_back;
	}

	void makeTransRate(const char * back, const char * forw,
		float weight) {
		HMMState * s_back = getState(back);
		HMMState * s_forw = getState(forw);
		if (NULL == s_back || NULL == s_forw) {
			return;
		}
		Connector * tran = new Connector(weight);
		if (NULL == tran) {
			return;
		}
		tran->back = s_back;
		tran->forw = s_forw;
		s_back->tran.insertLink(tran);
		s_forw->trans_back.insertLink(tran);
	}
};

class HMM {
public:
	HMM() :shown(0), hidden(0){
	}

	StateContainer shown;
	StateContainer hidden;

	void makeConfusionRate(const char * back, const char * forw,
		float weight) {
		HMMState * s_back = hidden.getState(back);
		HMMState * s_forw = shown.getState(forw);
		if (NULL == s_back || NULL == s_forw) {
			return;
		}
		Connector * tran;
		tran = new Connector(weight);
		if (NULL == tran) {
			return;
		}
		tran->back = s_back;
		tran->forw = s_forw;
		s_back->conf.insertLink(tran);
		s_forw->conf.insertLink(tran);
	}


	/////////////////////////////////////////
	//TEST functions
	void printCurrentRate() {
		//current rate
		printf("********current rate: \n");
		HMMState * state = this->hidden.link;
		if (state) {
			do {
				printf("%s(%.2f)\t", state->name, state->conn.weight);

				state = this->hidden.next(state);
			} while (state && state != this->hidden.link);
		}
		printf("\n");
	}

	void printTransitionRate() {
		//trans rate
		printf("********transition rate: \n");
		HMMState * state = this->hidden.link;
		if (state) {
			do {
				printf("%s->", state->name);
				Connector * tran = state->tran.link;
				if (tran) {
					do {
						printf("(%.2f)%s->",
							tran->weight, tran->forw->name);

						tran = state->tran.next(tran);
					} while (tran && tran != state->tran.link);
				}
				printf("\n");

				state = this->hidden.next(state);
			} while (state && state != this->hidden.link);
		}
	}
	void printTransitionBackRate() {
		//trans rate
		printf("********transition back rate: \n");
		HMMState * state = this->hidden.link;
		if (state) {
			do {
				printf("%s->", state->name);
				Connector * tran = state->trans_back.link;
				if (tran) {
					do {
						printf("(%.2f)%s->",
							tran->weight, tran->back->name);

						tran = state->trans_back.next(tran);
					} while (tran && tran != state->trans_back.link);
				}
				printf("\n");

				state = this->hidden.next(state);
			} while (state && state != this->hidden.link);
		}
	}

	void printObversableStates() {
		//shown
		printf("********observable states: \n");
		HMMState * state = this->shown.link;
		if (state) {
			do {
				printf("%s\t", state->name);

				state = this->shown.next(state);
			} while (state && state != this->shown.link);
		}
		printf("\n");
	}

	void printConfusionRate() {
		//confusion rate
		HMMState * state;
		printf("********hidden->observable confusion rate: \n");
		state = this->hidden.link;
		if (state) {
			do {
				printf("%s->", state->name);
				Connector * tran = state->conf.link;
				if (tran) {
					do {
						printf("(%.2f)%s->",
							tran->weight, tran->forw->name);

						tran = state->conf.next(tran);
					} while (tran && tran != state->conf.link);
				}
				printf("\n");

				state = this->hidden.next(state);
			} while (state && state != this->hidden.link);
		}
	} 
	void printConfusionBackRate() {
		//confusion back rate
		HMMState * state;
		printf("********observable->hidden confusion rate: \n");
		state = this->shown.link;
		if (state) {
			do {
				printf("%s->", state->name);
				Connector * tran = state->conf.link;
				if (tran) {
					do {
						printf("(%.2f)%s->",
							tran->weight, tran->back->name);

						tran = state->conf.next(tran);
					} while (tran && tran != state->conf.link);
				}
				printf("\n");

				state = this->shown.next(state);
			} while (state && state != this->shown.link);
		}
	}
};

#endif
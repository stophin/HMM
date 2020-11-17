//this->h
//

#ifndef _HMM_H_
#define _HMM_H_

#include "../common/MultiLinkList.h"

#define PRECISE_PERCENT 100

class HMMState;
class Connector {
public:
	Connector(EFTYPE v) :
		back(NULL),
		forw(NULL),
		weight(v){

		initialize();
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
#define Connector_Size 3
	INT uniqueID;
	Connector * prev[Connector_Size];
	Connector * next[Connector_Size];
	void initialize() {
		for (int i = 0; i < Connector_Size; i++) {
			this->prev[i] = NULL;
			this->next[i] = NULL;
		}
	}
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
	void clear() {
		void * _ptr = this;
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
		delete(this);
	}
};
class HMMState {
public:
	EFTYPE _buff[2];
	EFTYPE * input;
	EFTYPE * output;
	MultiLinkList<Connector> trace;
public:
	HMMState(const char * name, int linkindex = 0) :
		prob(NULL),
		conn(0), tran(0), conf(linkindex), trans_back(1), name(NULL), trace(2) {
		if (NULL == name) {
			return;
		}
		int len = strlen(name);
		this->name = new char[len + 1];
		memcpy(this->name, name, len);
		this->name[len] = 0;


		for (int i = 0; i < 2; i++) {
			_buff[i] = 0;
		}
		input = &_buff[0];
		output = &_buff[1];

		initialize();
	}
	void switchIO() {
		if (input == &_buff[0]) {
			input = &_buff[1];
			output = &_buff[0];
		}
		else {
			input = &_buff[0];
			output = &_buff[1];
		}
	}
	~HMMState() {
		if (this->name) {
			delete[] this->name;
			this->name = NULL;
		}
		tran.~MultiLinkList();
	}

	char *name;

	HMMState * prob;

	Connector conn;
	MultiLinkList<Connector> tran;
	MultiLinkList<Connector> trans_back;
	MultiLinkList<Connector> conf;
	// for multilinklist
#define HMMState_Size 2
	INT uniqueID;
	HMMState * prev[HMMState_Size];
	HMMState * next[HMMState_Size];
	void initialize() {
		for (int i = 0; i < HMMState_Size; i++) {
			this->prev[i] = NULL;
			this->next[i] = NULL;
		}
	}
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

	void clear() {
		void * _ptr = this;
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
		delete(this);
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
	HMMState *  addState(const char * name, int linkindex = 0) {
		if (getState(name)) {
			return NULL;
		}
		return addStateEx(name, linkindex);
	}
	HMMState *  addStateEx(const char * name, int linkindex = 0) {
		HMMState * state = new HMMState(name, linkindex);
		if (NULL == state) {
			return NULL;
		}
		this->insertLink(state);
		return state;
	}
	void makeRate(const char * back, const char * forw,
		EFTYPE w_back) {
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
		EFTYPE weight) {
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
	HMM() :shown(0), hidden(0), observe(1){
	}

	StateContainer shown;
	StateContainer hidden;
	StateContainer observe;

	void makeConfusionRate(const char * back, const char * forw,
		EFTYPE weight) {
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

	//add an obversable result
	//and hmm will calculate the probability
	//of each hidden state
	void addObversableResult(const char * res) {
		HMMState * sobserve = shown.getState(res);
		if (NULL == sobserve) {
			return;
		}
		HMMState * newobserve = NULL;
		if (observe.linkcount == 0) {
			newobserve = observe.addStateEx(res, 1);

			printf("%s->", sobserve->name);
			Connector * tran = sobserve->conf.link;
			if (tran) {
				do {

					//calculate probability
					*tran->back->output = tran->back->conn.weight * tran->weight;
					//for viterbi trace
					Connector * conn = new Connector(*tran->back->output);
					conn->back = NULL;
					conn->forw = tran->back;
					newobserve->trace.insertLink(conn);

					printf("(%.2e?%.2e:%.2e)%s->",
						tran->back->conn.weight * PRECISE_PERCENT, tran->weight * PRECISE_PERCENT, *tran->back->output * PRECISE_PERCENT, tran->back->name);


					tran = sobserve->conf.next(tran);
				} while (tran && tran != sobserve->conf.link);
				printf("\n");
			}
			//switch input&output
			tran = sobserve->conf.link;
			if (tran) {
				do {

					tran->back->switchIO();

					tran = sobserve->conf.next(tran);
				} while (tran && tran != sobserve->conf.link);
			}
		}
		else {
			newobserve = observe.addStateEx(res, 1);

			printf("%s->\n", sobserve->name);
			Connector * tran = sobserve->conf.link;
			if (tran) {
				do {

					//calculate probability
					*tran->back->output = tran->weight;

					EFTYPE val = 0;
					EFTYPE cur = 0;
					EFTYPE sum = 0;
					HMMState *maxval_prev = NULL;
					HMMState *maxval = NULL;
					printf("(%.2e)%s=>", tran->weight * PRECISE_PERCENT, tran->back->name);
					Connector * trans = tran->back->tran.link;
					if (trans) {
						do {

							printf("(%.2e * %.2e)%s->",
								*trans->forw->input * PRECISE_PERCENT, trans->weight * PRECISE_PERCENT, trans->forw->name);
							 cur = *trans->forw->input * trans->weight;
							 if (val < cur) {
								 val = cur;
								 maxval_prev = trans->forw;
								 maxval = trans->back;
							 }
							 sum += cur;

							trans = tran->back->tran.next(trans);
						} while (trans && trans != tran->back->tran.link);
					}
					*trans->back->output *= sum;
					//for viterbi trace
					Connector * conn = new Connector(tran->weight * val);
					conn->back = maxval_prev;
					conn->forw = maxval;
					newobserve->trace.insertLink(conn);

					printf("\n(%.2e * %.2e:%.2e)%s->\n",
						sum * PRECISE_PERCENT, tran->weight * PRECISE_PERCENT, *tran->back->output * PRECISE_PERCENT, tran->back->name);

					tran = sobserve->conf.next(tran);
				} while (tran && tran != sobserve->conf.link);
			}
			//switch input&output
			tran = sobserve->conf.link;
			if (tran) {
				do {

					tran->back->switchIO();

					tran = sobserve->conf.next(tran);
				} while (tran && tran != sobserve->conf.link);
			}
		}
		//display observe
		HMMState * state = observe.link;
		printf("Observation: ");
		if (state) {
			do {

				printf("%s->", state->name);

				state = observe.next(state);
			} while (state && state != observe.link);
		}
		printf("\n");
		//get max probability
		Connector * tran = sobserve->conf.link;
		Connector * maxTrans = NULL;
		if (tran) {
			do {

				if (maxTrans == NULL) {
					maxTrans = tran;
				}
				else if (*tran->back->input > *maxTrans->back->input) {
					maxTrans = tran;
				}
				printf("%s(%.2e)->", tran->back->name, *tran->back->input * PRECISE_PERCENT);

				tran = sobserve->conf.next(tran);
			} while (tran && tran != sobserve->conf.link);
		}
		printf("\n");
		if (NULL == maxTrans) {
			printf("No max\n");
		} else {
			newobserve->prob = maxTrans->back;
			printf("Max: %s(%.2e)\n", maxTrans->back->name, *maxTrans->back->input * PRECISE_PERCENT);
		}
	}

	static int match(const char * src, const char * dest, int strict = 0) {
		if (NULL == src) {
			return 0;
		}
		if (NULL == dest) {
			return 0;
		}
		int i;
		for (i = 0; src[i] && dest[i]; i++) {
			if (src[i] == dest[i]) {
				continue;
			}
			return 0;
		}
		if (strict) {
			if (src[i] != dest[i]) {
				return 0;
			}
		}
		return i;
	}


	/////////////////////////////////////////
	//TEST functions
	void printCurrentRate() {
		//current rate
		printf("********current rate: \n");
		HMMState * state = this->hidden.link;
		if (state) {
			do {
				printf("%s(%.2e)\t", state->name, state->conn.weight * PRECISE_PERCENT);

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
						printf("(%.2e)%s->",
							tran->weight * PRECISE_PERCENT, tran->forw->name);

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
						printf("(%.2e)%s->",
							tran->weight * PRECISE_PERCENT, tran->back->name);

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
						printf("(%.2e)%s->",
							tran->weight * PRECISE_PERCENT, tran->forw->name);

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
						printf("(%.2e)%s->",
							tran->weight * PRECISE_PERCENT, tran->back->name);

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
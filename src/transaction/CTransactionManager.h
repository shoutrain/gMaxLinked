/*
 ============================================================================
 Name        : CTransactionManager.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#ifndef _C_TRANSACTION_MANAGER_H_
#define _C_TRANSACTION_MANAGER_H_

#include "../common/CTimerManager.h"
#include "../common/CMutex.h"

class CTransaction;

#include <map>

typedef std::map<unsigned int, CTransaction *> TransactionMap;

class CTransactionManager: public CTimerManager {
public:
	static CTransactionManager *instance();

	void work();

	CTransaction *findTransaction(unsigned int id);
	bool registerTransaction(CTransaction *transaction);
	bool unregisterTransaction(CTransaction *transaction);

protected:
	bool onTimer(long unsigned int timerId, void *parameter);

private:
	CTransactionManager();
	virtual ~CTransactionManager();

	static CTransactionManager *_tm;

	TransactionMap _transactionMap;
	CMutex _mutex;
};

#endif // _C_TRANSACTION_MANAGER_H_

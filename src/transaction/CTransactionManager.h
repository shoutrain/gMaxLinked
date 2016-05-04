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

typedef std::map<ub4_, CTransaction *> TransactionMap;

class CTransactionManager: public CTimerManager {
public:
	static CTransactionManager *instance();

	none_ work();

	CTransaction *findTransaction(ub4_ id);
	bool_ registerTransaction(CTransaction *transaction);
	bool_ unregisterTransaction(CTransaction *transaction);

protected:
	bool_ onTimer(ub8_ timerId, obj_ parameter);

private:
	CTransactionManager();
	virtual ~CTransactionManager();

	static CTransactionManager *_tm;

	TransactionMap _transactionMap;
	CMutex _mutex;
};

#endif // _C_TRANSACTION_MANAGER_H_

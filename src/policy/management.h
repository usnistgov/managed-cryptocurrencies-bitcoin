
#ifndef BITCOIN_MANAGEMENT_H
#define BITCOIN_MANAGEMENT_H

#include <amount.h>

class CManagementPolicy
{
public:

	enum CPolicyChangeType {
		M_ROLE_ACTIVE		= 0,
		C_ROLE_ACTIVE		= 1,
		L_ROLE_ACTIVE		= 2,
		U_ROLE_ACTIVE		= 3,
		A_ROLE_ACTIVE		= 4,
		L_COIN_TRANSFER_ACTIVE	= 5,
		C_COIN_CREATION_LIMIT	= 6,
		BLOCK_REWARD_MODE	= 7,
		MANUAL_REWARD_VAL	= 8,
		MANUAL_REWARD_MIN	= 9,
		AUTO_REWARD_DECAY_VAL	= 10,
		AUTO_REWARD_DECAY_MAX	= 11,
		MINIMUM_TX_FEE		= 12,
		MNG_TX_PERIODICITY	= 13,
		MNG_TX_MIN_PER_PERIOD	= 14,
		NOOP			= 15,
	};

	enum {
		ACTION_ROLE_ADD = 0b1,
		ACTION_ROLE_REM = 0b0,
	};

	struct CActivePolicy {
		bool mRoleActive		= true;
		bool cRoleActive		= true;
		bool lRoleActive		= true;
		bool uRoleActive		= true;
		bool aRoleActive		= true;
		bool lCoinTransferActive	= true;
		CAmount cCoinCreationLimit	= 0;
		bool blockRewardMode		= 0;
		CAmount manualRewardVal		= 50 * COIN;
		CAmount manualRewardMin		= 0;
		float autoRewardDecayVal	= 0.5;
		float autoRewardDecayMax	= 1.0;
		CAmount minimumTXFee		= 3000;
		unsigned int mng_TXPeriodicity	= 0;
		unsigned int mngTXMinPerPeriod	= 0;
	} activePolicy;

	CAmount GetGenesisReward() const
	{
		return CAmount(	ACTION_ROLE_ADD,
				this->activePolicy.mRoleActive,
				this->activePolicy.cRoleActive,
				this->activePolicy.lRoleActive,
				this->activePolicy.uRoleActive,
				this->activePolicy.aRoleActive);
	}

	CActivePolicy& GetActivePolicy() {
		return this->activePolicy;
	}
};


#endif //  BITCOIN_MANAGEMENT_H

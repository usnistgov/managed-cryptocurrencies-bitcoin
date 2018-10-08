
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

	struct CActivePolicy {
		bool M_role_active			= true;
		bool C_role_active			= true;
		bool L_role_active			= true;
		bool U_role_active			= true;
		bool A_role_active			= true;
		bool L_coin_transfer_active		= true;
		CAmount C_coin_creation_limit		= 0;
		bool block_reward_mode			= 0;
		CAmount manual_reward_val		= 50 * COIN;
		CAmount manual_reward_min		= 0;
		float auto_reward_decay_val		= 0.5;
		float auto_reward_decay_max		= 1.0;
		CAmount minimum_tx_fee			= 3000;
		unsigned int mng_tx_periodicity		= 0;
		unsigned int mng_tx_min_per_period	= 0;
	} activePolicy;
};


#endif //  BITCOIN_MANAGEMENT_H

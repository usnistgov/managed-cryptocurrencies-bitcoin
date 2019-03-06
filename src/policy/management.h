#ifndef BITCOIN_MANAGEMENT_H
#define BITCOIN_MANAGEMENT_H

#include <amount.h>
#include <primitives/transaction.h>

class CManagementPolicy
{
public:

    enum CPolicyChangeType {
        NOOP                       = 0b1111111111111111111111111111111,
        ACTIVATE_ROLE_M            = 0,
        ACTIVATE_ROLE_C            = 1,
        ACTIVATE_ROLE_L            = 2,
        ACTIVATE_ROLE_U            = 3,
        ACTIVATE_ROLE_A            = 4,
        ACTIVATE_ROLE_L_TRANSFER   = 5,
        SET_ROLE_C_CREATION_LIMIT  = 6,
        SET_BLOCK_REWARD_MODE      = 7,
        SET_CUR_BLOCK_REWARD       = 8,
        SET_MIN_BLOCK_REWARD       = 9,
        SET_CUR_BLOCK_REWARD_DECAY = 10,
        SET_MAX_BLOCK_REWARD_DECAY = 11,
        SET_MIN_TX_FEE             = 12,
        SET_MNG_TX_PERIODICITY     = 13,
        SET_MNG_TX_MIN_PER_PERIOD  = 14,
    };

    struct CActivePolicy {
        bool fRoleMActive               = true;
        bool fRoleCActive               = true;
        bool fRoleLActive               = true;
        bool fRoleRActive               = true;
        bool fRoleAActive               = true;
        bool fRoleDActive               = true;
        bool fRoleLCanMoveCoin          = true;
        CAmount nRoleCCoinCreationLimit = 1000 * COIN;
        bool fBlockRewardAuto           = true;
        CAmount nCurBlockReward         = 50 * COIN;
        CAmount nMinBlockReward         = 0;
        float nCurBlockRewardDecay      = 0.5;
        float nMaxBlockRewardDecay      = 1.0;
        CAmount nMinTxFee               = 3000;
        int nManagementTxPeriodicity    = 0;
        int nManagementTxMinPerPeriod   = 0;
    } activePolicy;

    CActivePolicy GetActivePolicy() const {
        return activePolicy;
    }

    CAmount GetCurrentReward() const {
        return activePolicy.nCurBlockReward;
    }

    CAmount GetCoinCreationLimit() const {
        return activePolicy.nRoleCCoinCreationLimit;
    }

    CRoleChangeMode GetActiveRoles() const {
      CRoleChangeMode role;
      role.fRoleM = activePolicy.fRoleMActive;
      role.fRoleC = activePolicy.fRoleCActive;
      role.fRoleL = activePolicy.fRoleLActive;
      role.fRoleR = activePolicy.fRoleRActive;
      role.fRoleA = activePolicy.fRoleAActive;
      role.fRoleD = activePolicy.fRoleDActive;
      role.nReserved = CRoleChangeMode::NULL_ROLE_RESERVED;
      return role;
    }
};

#endif // BITCOIN_MANAGEMENT_H

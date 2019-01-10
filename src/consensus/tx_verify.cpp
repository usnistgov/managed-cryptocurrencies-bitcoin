// Copyright (c) 2017-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/tx_verify.h>

#include <consensus/consensus.h>
#include <primitives/transaction.h>
#include <pubkey.h>
#include <script/script.h>
#include <script/standard.h>
#include <script/interpreter.h>
#include <consensus/validation.h>

// TODO remove the following dependencies
#include <chain.h>
#include <coins.h>
#include <utilmoneystr.h>

bool IsFinalTx(const CTransaction &tx, int nBlockHeight, int64_t nBlockTime)
{
    if (tx.nLockTime == 0)
        return true;
    if ((int64_t)tx.nLockTime < ((int64_t)tx.nLockTime < LOCKTIME_THRESHOLD ? (int64_t)nBlockHeight : nBlockTime))
        return true;
    for (const auto& txin : tx.vin) {
        if (!(txin.nSequence == CTxIn::SEQUENCE_FINAL))
            return false;
    }
    return true;
}

std::pair<int, int64_t> CalculateSequenceLocks(const CTransaction &tx, int flags, std::vector<int>* prevHeights, const CBlockIndex& block)
{
    assert(prevHeights->size() == tx.vin.size());

    // Will be set to the equivalent height- and time-based nLockTime
    // values that would be necessary to satisfy all relative lock-
    // time constraints given our view of block chain history.
    // The semantics of nLockTime are the last invalid height/time, so
    // use -1 to have the effect of any height or time being valid.
    int nMinHeight = -1;
    int64_t nMinTime = -1;

    // tx.nVersion is signed integer so requires cast to unsigned otherwise
    // we would be doing a signed comparison and half the range of nVersion
    // wouldn't support BIP 68.
    bool fEnforceBIP68 = static_cast<uint32_t>(tx.nVersion) >= 2
                      && flags & LOCKTIME_VERIFY_SEQUENCE;

    // Do not enforce sequence numbers as a relative lock time
    // unless we have been instructed to
    if (!fEnforceBIP68) {
        return std::make_pair(nMinHeight, nMinTime);
    }

    for (size_t txinIndex = 0; txinIndex < tx.vin.size(); txinIndex++) {
        const CTxIn& txin = tx.vin[txinIndex];

        // Sequence numbers with the most significant bit set are not
        // treated as relative lock-times, nor are they given any
        // consensus-enforced meaning at this point.
        if (txin.nSequence & CTxIn::SEQUENCE_LOCKTIME_DISABLE_FLAG) {
            // The height of this input is not relevant for sequence locks
            (*prevHeights)[txinIndex] = 0;
            continue;
        }

        int nCoinHeight = (*prevHeights)[txinIndex];

        if (txin.nSequence & CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG) {
            int64_t nCoinTime = block.GetAncestor(std::max(nCoinHeight-1, 0))->GetMedianTimePast();
            // NOTE: Subtract 1 to maintain nLockTime semantics
            // BIP 68 relative lock times have the semantics of calculating
            // the first block or time at which the transaction would be
            // valid. When calculating the effective block time or height
            // for the entire transaction, we switch to using the
            // semantics of nLockTime which is the last invalid block
            // time or height.  Thus we subtract 1 from the calculated
            // time or height.

            // Time-based relative lock-times are measured from the
            // smallest allowed timestamp of the block containing the
            // txout being spent, which is the median time past of the
            // block prior.
            nMinTime = std::max(nMinTime, nCoinTime + (int64_t)((txin.nSequence & CTxIn::SEQUENCE_LOCKTIME_MASK) << CTxIn::SEQUENCE_LOCKTIME_GRANULARITY) - 1);
        } else {
            nMinHeight = std::max(nMinHeight, nCoinHeight + (int)(txin.nSequence & CTxIn::SEQUENCE_LOCKTIME_MASK) - 1);
        }
    }

    return std::make_pair(nMinHeight, nMinTime);
}

bool EvaluateSequenceLocks(const CBlockIndex& block, std::pair<int, int64_t> lockPair)
{
    assert(block.pprev);
    int64_t nBlockTime = block.pprev->GetMedianTimePast();
    if (lockPair.first >= block.nHeight || lockPair.second >= nBlockTime)
        return false;

    return true;
}

bool SequenceLocks(const CTransaction &tx, int flags, std::vector<int>* prevHeights, const CBlockIndex& block)
{
    return EvaluateSequenceLocks(block, CalculateSequenceLocks(tx, flags, prevHeights, block));
}

unsigned int GetLegacySigOpCount(const CTransaction& tx)
{
    unsigned int nSigOps = 0;
    for (const auto& txin : tx.vin)
    {
        nSigOps += txin.scriptSig.GetSigOpCount(false);
    }
    for (const auto& txout : tx.vout)
    {
        nSigOps += txout.scriptPubKey.GetSigOpCount(false);
    }
    return nSigOps;
}

unsigned int GetP2SHSigOpCount(const CTransaction& tx, const CCoinsViewCache& inputs)
{
    if (tx.IsCoinBase())
        return 0;

    unsigned int nSigOps = 0;
    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
        const Coin& coin = inputs.AccessCoin(tx.vin[i].prevout);
        assert(!coin.IsSpent());
        const CTxOut &prevout = coin.out;
        if (prevout.scriptPubKey.IsPayToScriptHash())
            nSigOps += prevout.scriptPubKey.GetSigOpCount(tx.vin[i].scriptSig);
    }
    return nSigOps;
}

int64_t GetTransactionSigOpCost(const CTransaction& tx, const CCoinsViewCache& inputs, int flags)
{
    int64_t nSigOps = GetLegacySigOpCount(tx) * WITNESS_SCALE_FACTOR;

    if (tx.IsCoinBase())
        return nSigOps;

    if (flags & SCRIPT_VERIFY_P2SH) {
        nSigOps += GetP2SHSigOpCount(tx, inputs) * WITNESS_SCALE_FACTOR;
    }

    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
        const Coin& coin = inputs.AccessCoin(tx.vin[i].prevout);
        assert(!coin.IsSpent());
        const CTxOut &prevout = coin.out;
        nSigOps += CountWitnessSigOps(tx.vin[i].scriptSig, prevout.scriptPubKey, &tx.vin[i].scriptWitness, flags);
    }
    return nSigOps;
}

bool CheckTransaction(const CTransaction& tx, CValidationState &state, bool fCheckDuplicateInputs)
{
    // Basic checks that don't depend on any context
    if (tx.vin.empty())
        return state.DoS(10, false, REJECT_INVALID, "bad-txns-vin-empty");
    if (tx.vout.empty())
        return state.DoS(10, false, REJECT_INVALID, "bad-txns-vout-empty");
    // Size limits (this doesn't take the witness into account, as that hasn't been checked for malleability)
    if (::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS) * WITNESS_SCALE_FACTOR > MAX_BLOCK_WEIGHT)
        return state.DoS(100, false, REJECT_INVALID, "bad-txns-oversize");

    switch (tx.nVersion) {
        case CTransaction::VERSION_COINBASE_TRANSFER:
            // TODO
            break;
        case CTransaction::VERSION_COIN_TRANSFER:
        {
            // Check if the first vout is a "role repeat"
            if (tx.vout[0].nTxType != CTxOut::ROLE_CHANGE)
                return state.DoS(100, false, REJECT_INVALID, "bad-txns-vout-wrong-type");

            // Check for negative or overflow output values
            CAmount nValueOut = 0;
            // Start at offset 1 because the first vout is a "role repeat"
            for (size_t idx = 1; idx < tx.vout.size(); ++idx)
            {
                if (tx.vout[idx].nTxType != CTxOut::COIN_TRANSFER)
                    return state.DoS(100, false, REJECT_INVALID, "bad-txns-vout-wrong-type");
                CAmount nValue = tx.vout[idx].nValue;
                if (nValue < 0)
                    return state.DoS(100, false, REJECT_INVALID, "bad-txns-vout-negative");
                if (nValue > MAX_MONEY)
                    return state.DoS(100, false, REJECT_INVALID, "bad-txns-vout-toolarge");
                nValueOut += nValue;
                if (!MoneyRange(nValueOut))
                    return state.DoS(100, false, REJECT_INVALID, "bad-txns-txouttotal-toolarge");
            }
            break;
        }
        case CTransaction::VERSION_ROLE_CHANGE:
            // TODO
            break;
        case CTransaction::VERSION_POLICY_CHANGE:
            // TODO
            break;
        case CTransaction::VERSION_ROLE_CHANGE_FEE:
            // TODO
            break;
        case CTransaction::VERSION_POLICY_CHANGE_FEE:
            // TODO
            break;
        default:
            return state.DoS(100, false, REJECT_INVALID, "bad-txns-version");
    }

    // Check for duplicate inputs - note that this check is slow so we skip it in CheckBlock
    if (fCheckDuplicateInputs) {
        std::set<COutPoint> vInOutPoints;
        for (const auto& txin : tx.vin)
        {
            if (!vInOutPoints.insert(txin.prevout).second)
                return state.DoS(100, false, REJECT_INVALID, "bad-txns-inputs-duplicate");
        }
    }

    if (tx.IsCoinBase())
    {
        if (tx.vin[0].scriptSig.size() < 2 || tx.vin[0].scriptSig.size() > 100)
            return state.DoS(100, false, REJECT_INVALID, "bad-cb-length");
    }
    else
    {
        for (const auto& txin : tx.vin)
            if (txin.prevout.IsNull())
                return state.DoS(10, false, REJECT_INVALID, "bad-txns-prevout-null");
    }

    return true;
}

/**
 * Check role validity.
 *
 * @param nRole
 * @return
 */
bool isValidRoleIn(const CRoleChangeMode& nRoleIn)
{
    // Account must be registered and not disabled
    if(!nRoleIn.fRoleR || nRoleIn.fRoleD) {
        return false;
    }

    // Account must only have one role
    if (nRoleIn.fRoleM && !nRoleIn.fRoleC && !nRoleIn.fRoleL && !nRoleIn.fRoleA) {
        // Role M+R only
        return true;
    }
    if (nRoleIn.fRoleC && !nRoleIn.fRoleM && !nRoleIn.fRoleL && !nRoleIn.fRoleA) {
        // Role C+R only
        return true;
    }
    if (nRoleIn.fRoleL && !nRoleIn.fRoleC && !nRoleIn.fRoleM && !nRoleIn.fRoleA) {
        // Role L+R only
        return true;
    }
    if (nRoleIn.fRoleA && !nRoleIn.fRoleC && !nRoleIn.fRoleL && !nRoleIn.fRoleM) {
        // Role A+R only
        return true;
    }
    if (!nRoleIn.fRoleM && !nRoleIn.fRoleC && !nRoleIn.fRoleL && !nRoleIn.fRoleA) {
        // Role R only
        return true;
    }

    // By default
    return false;
}

bool isAuthorizedRCM(const CRoleChangeMode& inRole, const CRoleChangeMode& outRole)
{
    // TODO: retrieve the old role if any to calculate the delta between the old roles and the new

    // Managers (M role) can perform any role change.
    if (inRole.fRoleM) {
        return true;
    }

    // Account managers (A role) can register users.
    if (outRole.fRoleR && inRole.fRoleA) {
        return true;
    }

    // Law enforcement (L role) can disable accounts.
    if (outRole.fRoleD && inRole.fRoleL) {
        return true;
    }

    // By default
    return false;
}

/**
 * Check authorization to execute a given transaction.
 *
 * @param nVersion
 * @param vinAuth
 * @param txVouts
 * @return
 */
bool isAuthorized(int32_t nVersion, const CRoleChangeMode& inRole, const std::vector<CTxOut>& txVouts)
{
    // Check role validity
    if (!isValidRoleIn(inRole)) {
        return false;
    }

    // Managers can perform anything (validity check made sure that they are registered and not disabled)
    if (inRole.fRoleM) {
        return true;
    }

    switch(nVersion)
    {
        case CTransaction::VERSION_COINBASE_TRANSFER:
            // TODO
            break;
        case CTransaction::VERSION_COIN_TRANSFER:
            // The sender needs at least role R, but it's already checked for in isValidRoleIn
            return true;
        case CTransaction::VERSION_ROLE_CHANGE:
            // Check each vout to ensure correct vin role
            for(const CTxOut& vout: txVouts) {
                
                if(!isAuthorizedRCM(inRole, vout.nRole)) {
                    return false;
                }
            }
            return true;
        case CTransaction::VERSION_ROLE_CHANGE_FEE:
            // Check each vout to ensure correct vin role.
            // Note: the first vout correspond to the change and should not be checked
            for(unsigned int i = 1; i < txVouts.size(); ++i) {
                const CTxOut& vout = txVouts[i];

                if (!isAuthorizedRCM(inRole, vout.nRole)) {
                    return false;
                }
            }
            break;
        case CTransaction::VERSION_POLICY_CHANGE:
            // TODO
            break;
        case CTransaction::VERSION_POLICY_CHANGE_FEE:
            // TODO
            break;
        default:
            throw std::ios_base::failure(std::string(__func__) + ":" + std::to_string(__LINE__) + "> Unknown transaction version: " + std::to_string(nVersion)); // FIXME
            return false;
    }

    // By default
    return false;
}

bool Consensus::CheckTxInputs(const CTransaction& tx, CValidationState& state, const CCoinsViewCache& inputs, int nSpendHeight, CAmount& txfee)
{
    // are the actual inputs available?
    if (!inputs.HaveInputs(tx)) {
        return state.DoS(100, false, REJECT_INVALID, "bad-txns-inputs-missingorspent", false,
                         strprintf("%s: inputs missing/spent", __func__));
    }

    // TODO: We might need special handling for "coinbase change tx"

    // Retrieve the first vin's utxo
    const COutPoint &prevout = tx.vin[0].prevout;
    const Coin& coin = inputs.AccessCoin(prevout);

    // Assert that the first vin points to a role change utxo
    // TODO: "coinbase change tx" need special handling here
    if (coin.out.nTxType != CTxOut::ROLE_CHANGE) {
        return state.Invalid(false, REJECT_INVALID, "bad-txns-first-vin-not-roles");
    }

    // Assert that the first vout is a "role repeat"
    // TODO: "coinbase change tx" need special handling here
    if (tx.vout[0].nTxType != CTxOut::ROLE_CHANGE) {
        return state.Invalid(false, REJECT_INVALID, "bad-txns-first-vout-not-roles");
    }

    // Ensure that the account has sufficient privileges to perform the operation
    if(!isAuthorized(tx.nVersion, coin.out.nRole, tx.vout)) {
        return state.Invalid(false, REJECT_INVALID, "bad-txns-not-authorized");
    }

    // FIXME Add further validation

    // Ensure that all vins are using the same address, so that one cannot use its privileges with another address
    // Also ensure that all vins except the first are coin transfer utxo
    CTxDestination dest1, dest2;
    assert(ExtractDestination(coin.out.scriptPubKey, dest1));
    for (size_t i = 1; i < tx.vin.size(); ++i) {
        const COutPoint &prevout2 = tx.vin[i].prevout;
        const Coin& coin2 = inputs.AccessCoin(prevout2);
        if (coin2.out.nTxType != CTxOut::COIN_TRANSFER) // FIXME: Perhaps add COINBASE_TRANSFER?
            return state.Invalid(false, REJECT_INVALID, "bad-txns-following-vin-not-coin");
        assert(ExtractDestination(coin2.out.scriptPubKey, dest2));
        if (dest1 != dest2)
            return state.Invalid(false, REJECT_INVALID, "bad-txns-address-mismatch");
    }

    // Ensure that the first vout has the same address as the first vin ("role repeat")
    assert(ExtractDestination(tx.vout[0].scriptPubKey, dest2));
    if (dest1 != dest2)
        return state.Invalid(false, REJECT_INVALID, "bad-txns-address-mismatch");
    
    // Check the value of the "role repeat" vout
    switch (tx.nVersion)
    {
        case CTransaction::VERSION_ROLE_CHANGE:
        case CTransaction::VERSION_ROLE_CHANGE_FEE:
            // A user is allowed to drop its privileges to attach itself to a new parent
            if (tx.vout[0].nRole == CRoleChangeMode())
                break;
            // Fallthrough
        case CTransaction::VERSION_COIN_TRANSFER:
        case CTransaction::VERSION_POLICY_CHANGE:
        case CTransaction::VERSION_POLICY_CHANGE_FEE:
            // If the "role repeat" is the same as the current role, we're good
            if (tx.vout[0].nRole == coin.out.nRole)
                break;
            return state.Invalid(false, REJECT_INVALID, "bad-txns-invalid-rolerepeat");
        case CTransaction::VERSION_COINBASE_TRANSFER:
            // No "role repeat" for this tx type
            break;
        default:
            return state.Invalid(false, REJECT_INVALID, "bad-txns-invalid-txversion");
    }

    // Calculate fees
    switch (tx.nVersion)
    {
        case CTransaction::VERSION_ROLE_CHANGE:
        case CTransaction::VERSION_POLICY_CHANGE:
            // Free role/policy change transactions don't require a fee
            // FIXME Make sure that the current policy allows for these transaction, but perhaps elsewhere
            txfee = 0;
            break;
        case CTransaction::VERSION_COINBASE_TRANSFER:
        case CTransaction::VERSION_COIN_TRANSFER:
        case CTransaction::VERSION_ROLE_CHANGE_FEE:
        case CTransaction::VERSION_POLICY_CHANGE_FEE:
        {
            CAmount nValueIn = 0;

            // Check that the change address is the same as the vin address
            assert(tx.vout.size() > 1);
            assert(ExtractDestination(tx.vout[1].scriptPubKey, dest2));
            if (dest1 != dest2)
                return state.Invalid(false, REJECT_INVALID, "bad-txns-address-mismatch");

            // Check the amount of coins used as input
            for (size_t i = 0; i < tx.vin.size(); ++i) {

                const COutPoint &prevout = tx.vin[i].prevout;
                const Coin& coin = inputs.AccessCoin(prevout);

                if (coin.out.nTxType != CTxOut::COIN_TRANSFER)
                    // Skip non-coin transfer vins
                    continue;

                assert(!coin.IsSpent());

                // If prev is coinbase, check that it's matured
                if (coin.IsCoinBase() && nSpendHeight - coin.nHeight < COINBASE_MATURITY) {
                    return state.Invalid(false,
                    REJECT_INVALID, "bad-txns-premature-spend-of-coinbase",
                    strprintf("tried to spend coinbase at depth %d", nSpendHeight - coin.nHeight));
                }

                // Check for negative or overflow input values
                nValueIn += coin.out.nValue;
                if (!MoneyRange(coin.out.nValue) || !MoneyRange(nValueIn)) {
                    return state.DoS(100, false, REJECT_INVALID, "bad-txns-inputvalues-outofrange");
                }
            }

            // Compare the amount used as inputs to the amount used as outputs
            const CAmount value_out = tx.GetValueOut();
            if (nValueIn < value_out) {
                return state.DoS(100, false, REJECT_INVALID, "bad-txns-in-belowout", false,
                        strprintf("value in (%s) < value out (%s)", FormatMoney(nValueIn), FormatMoney(value_out)));
            }

            // Tally transaction fees
            const CAmount txfee_aux = nValueIn - value_out;
            if (!MoneyRange(txfee_aux)) {
                return state.DoS(100, false, REJECT_INVALID, "bad-txns-fee-outofrange");
            }

            txfee = txfee_aux;
            break;
        }
        default:
            return state.Invalid(false, REJECT_INVALID, "bad-txns-invalid-txversion");
    }

    return true;
}

// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/transaction.h>

#include <hash.h>
#include <util.h>
#include <tinyformat.h>
#include <utilstrencodings.h>
#include <policy/management.h>

std::string COutPoint::ToString() const
{
    return strprintf("COutPoint(%s, %u)", hash.ToString().substr(0,10), n);
}

CTxIn::CTxIn(COutPoint prevoutIn, CScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = prevoutIn;
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

CTxIn::CTxIn(uint256 hashPrevTx, uint32_t nOut, CScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = COutPoint(hashPrevTx, nOut);
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

std::string CTxIn::ToString() const
{
    std::string str;
    str += "CTxIn(";
    str += prevout.ToString();
    if (prevout.IsNull())
        str += strprintf(", coinbase %s", HexStr(scriptSig));
    else
        str += strprintf(", scriptSig=%s", HexStr(scriptSig).substr(0, 24));
    if (nSequence != SEQUENCE_FINAL)
        str += strprintf(", nSequence=%u", nSequence);
    str += ")";
    return str;
}

CTxOut::CTxOut(const CAmount& nValueIn, CScript scriptPubKeyIn)
{
    nTxType = COIN_TRANSFER;
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
}

CTxOut::CTxOut(
        const bool fRoleMIn,
        const bool fRoleCIn,
        const bool fRoleLIn,
        const bool fRoleUIn,
        const bool fRoleAIn,
        CScript scriptPubKeyIn)
{
    nTxType = ROLE_CHANGE;
    nRole.fRoleM = fRoleMIn;
    nRole.fRoleC = fRoleCIn;
    nRole.fRoleL = fRoleLIn;
    nRole.fRoleU = fRoleUIn;
    nRole.fRoleA = fRoleAIn;
    nRole.nReserved = NULL_ROLE_RESERVED;
    scriptPubKey = scriptPubKeyIn;
}

CTxOut::CTxOut(const CRoleChangeMode& nRolesIn, CScript scriptPubKeyIn)
{
    nTxType = ROLE_CHANGE;
    nRole = nRolesIn;
    scriptPubKey = scriptPubKeyIn;
}

CTxOut::CTxOut(
        const bool fPermanentIn,
        const uint32_t nTypeIn,
        const uint32_t nParamIn,
        CScript scriptPubKeyIn)
{
    nTxType = POLICY_CHANGE;
    nPolicy.fPrmnt = fPermanentIn;
    nPolicy.nType  = nTypeIn;
    nPolicy.nParam = nParamIn;
    scriptPubKey = scriptPubKeyIn;
}

void CTxOut::SetNull()
{
    scriptPubKey.clear();
    switch(nTxType) {
        case ROLE_CHANGE:
            nRole.fRoleM = false;
            nRole.fRoleC = false;
            nRole.fRoleL = false;
            nRole.fRoleU = false;
            nRole.fRoleA = false;
            nRole.nReserved = CTxOut::NULL_ROLE_RESERVED;
            break;
        case POLICY_CHANGE:
            nPolicy.fPrmnt = false;
            nPolicy.nType = CManagementPolicy::NOOP;
            nPolicy.nParam = CTxOut::NULL_POLICY_PARAM;
        case COIN_TRANSFER:
        default:
            nValue = -1;
            break;
    }
}

#include <csignal>

bool CTxOut::IsNull() const
{
    switch(nTxType) {
        case COIN_TRANSFER:
            return (nValue == -1);
        case ROLE_CHANGE:
            return !nRole.fRoleM && !nRole.fRoleC && !nRole.fRoleL && !nRole.fRoleU && !nRole.fRoleA && (nRole.nReserved == NULL_ROLE_RESERVED);
        case POLICY_CHANGE:
            return !nPolicy.fPrmnt && (nPolicy.nType == CManagementPolicy::NOOP) && (nPolicy.nParam == NULL_POLICY_PARAM);
        default:
            LogPrint(BCLog::EXPERIMENT, "CTXout of invalid tx type: %u", nTxType);
            return true;
    }
}

std::string CTxOut::ToString() const
{
    switch(nTxType) {
        case COIN_TRANSFER:
            return strprintf("CTxOut(nValue=%d.%08d, scriptPubKey=%s)", nValue / COIN, nValue % COIN, HexStr(scriptPubKey).substr(0, 30));
        case ROLE_CHANGE:
            return strprintf("CTxOut(nRole=%c%c%c%c%c, scriptPubKey=%s)",
                    nRole.fRoleM ? 'M' : '.',
                    nRole.fRoleC ? 'C' : '.',
                    nRole.fRoleL ? 'L' : '.',
                    nRole.fRoleU ? 'U' : '.',
                    nRole.fRoleA ? 'A' : '.',
                    HexStr(scriptPubKey).substr(0, 30));
        case POLICY_CHANGE:
            return strprintf("CTxOut(fPrmnt=%s, nType=%u, nParam=%u, scriptPubKey=%s)",
                    nPolicy.fPrmnt ? "permanent" : "provisional",
                    nPolicy.nType, nPolicy.nParam,
                    HexStr(scriptPubKey).substr(0, 30));
        default:
            return strprintf("CTxOut(Invalid tx type: %u)", nTxType);
    }
}


CMutableTransaction::CMutableTransaction() : nVersion(CTransaction::CURRENT_VERSION), nLockTime(0) {}
CMutableTransaction::CMutableTransaction(const CTransaction& tx) : vin(tx.vin), vout(tx.vout), nVersion(tx.nVersion), nLockTime(tx.nLockTime) {}

uint256 CMutableTransaction::GetHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

uint256 CTransaction::ComputeHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

uint256 CTransaction::GetWitnessHash() const
{
    if (!HasWitness()) {
        return GetHash();
    }
    return SerializeHash(*this, SER_GETHASH, 0);
}

/* For backward compatibility, the hash is initialized to 0. TODO: remove the need for this default constructor entirely. */
CTransaction::CTransaction() : vin(), vout(), nVersion(CTransaction::CURRENT_VERSION), nLockTime(0), hash() {}
CTransaction::CTransaction(const CMutableTransaction &tx) : vin(tx.vin), vout(tx.vout), nVersion(tx.nVersion), nLockTime(tx.nLockTime), hash(ComputeHash()) {}
CTransaction::CTransaction(CMutableTransaction &&tx) : vin(std::move(tx.vin)), vout(std::move(tx.vout)), nVersion(tx.nVersion), nLockTime(tx.nLockTime), hash(ComputeHash()) {}

CAmount CTransaction::GetValueOut() const
{
    CAmount nValueOut = 0;
    for (const auto& tx_out : vout) {
        nValueOut += tx_out.nValue;
        if (!MoneyRange(tx_out.nValue) || !MoneyRange(nValueOut))
            throw std::runtime_error(std::string(__func__) + ": value out of range");
    }
    return nValueOut;
}

unsigned int CTransaction::GetTotalSize() const
{
    return ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
}

std::string CTransaction::ToString() const
{
    std::string str;
    str += strprintf("CTransaction(hash=%s, ver=%d, vin.size=%u, vout.size=%u, nLockTime=%u)\n",
        GetHash().ToString().substr(0,10),
        nVersion,
        vin.size(),
        vout.size(),
        nLockTime);
    for (const auto& tx_in : vin)
        str += "    " + tx_in.ToString() + "\n";
    for (const auto& tx_in : vin)
        str += "    " + tx_in.scriptWitness.ToString() + "\n";
    for (const auto& tx_out : vout)
        str += "    " + tx_out.ToString() + "\n";
    return str;
}

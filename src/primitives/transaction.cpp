// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/transaction.h>

#include <hash.h>
#include <util.h>
#include <base58.h>
#include <tinyformat.h>
#include <script/standard.h>
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
    Stack(__func__, __LINE__);
}

CTxOut::CTxOut(
        const bool fRoleMIn,
        const bool fRoleCIn,
        const bool fRoleLIn,
        const bool fRoleRIn,
        const bool fRoleAIn,
        const bool fRoleDIn,
        CScript scriptPubKeyIn)
{
    nTxType = ROLE_CHANGE;
    nRole.fRoleM = fRoleMIn;
    nRole.fRoleC = fRoleCIn;
    nRole.fRoleL = fRoleLIn;
    nRole.fRoleR = fRoleRIn;
    nRole.fRoleA = fRoleAIn;
    nRole.fRoleD = fRoleDIn;
    nRole.nReserved = NULL_ROLE_RESERVED;
    scriptPubKey = scriptPubKeyIn;
    Stack(__func__, __LINE__);
}

CTxOut::CTxOut(const CRoleChangeMode& nRolesIn, CScript scriptPubKeyIn)
{
    nTxType = ROLE_CHANGE;
    nRole = nRolesIn;
    scriptPubKey = scriptPubKeyIn;
    Stack(__func__, __LINE__);
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
    Stack(__func__, __LINE__);
}

void CTxOut::SetNull()
{
    nTxType = UNINITIALIZED;
    scriptPubKey.clear();
    nValue = -1;
}

#include <csignal>

bool CTxOut::IsNull() const
{
    return nTxType == UNINITIALIZED && nValue == -1;
}

std::string CTxOut::ToString() const
{
    CTxDestination dest;
    std::string sdest;
    if (ExtractDestination(scriptPubKey, dest))
        sdest = EncodeDestination(dest);
    else sdest = "unknown";
    switch(nTxType) {
        case COIN_TRANSFER:
            return strprintf("CTxOut(nValue=%d.%08d, dest=%s)", nValue / COIN, nValue % COIN, sdest);
        case ROLE_CHANGE:
            return strprintf("CTxOut(nRole=%c%c%c%c%c%c, dest=%s)",
                    nRole.fRoleM ? 'M' : '.',
                    nRole.fRoleC ? 'C' : '.',
                    nRole.fRoleL ? 'L' : '.',
                    nRole.fRoleR ? 'R' : '.',
                    nRole.fRoleA ? 'A' : '.',
                    nRole.fRoleD ? 'D' : '.',
                    sdest);
        case POLICY_CHANGE:
            return strprintf("CTxOut(fPrmnt=%s, nType=%u, nParam=%u, dest=%s)",
                    nPolicy.fPrmnt ? "permanent" : "provisional",
                    nPolicy.nType, nPolicy.nParam,
                    sdest);
        case UNINITIALIZED:
            return strprintf("CTxOut(Uninitialized)");
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
	    if (tx_out.nTxType == CTxOut::COIN_TRANSFER) {
            nValueOut += tx_out.nValue;
            if (!MoneyRange(tx_out.nValue) || !MoneyRange(nValueOut))
                throw std::runtime_error(std::string(__func__) + ": value out of range");
	    }
        else tx_out.Check(__func__, __LINE__); // FIXME
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

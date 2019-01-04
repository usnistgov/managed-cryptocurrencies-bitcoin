// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_TRANSACTION_H
#define BITCOIN_PRIMITIVES_TRANSACTION_H

#include <stdint.h>
#include <amount.h>
#include <script/script.h>
#include <serialize.h>
#include <uint256.h>

static const int SERIALIZE_TRANSACTION_NO_WITNESS = 0x40000000;

/** An outpoint - a combination of a transaction hash and an index n into its vout */
class COutPoint
{
public:
    uint256 hash;
    uint32_t n;

    COutPoint(): n((uint32_t) -1) { }
    COutPoint(const uint256& hashIn, uint32_t nIn): hash(hashIn), n(nIn) { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(hash);
        READWRITE(n);
    }

    void SetNull() { hash.SetNull(); n = (uint32_t) -1; }
    bool IsNull() const { return (hash.IsNull() && n == (uint32_t) -1); }

    friend bool operator<(const COutPoint& a, const COutPoint& b)
    {
        int cmp = a.hash.Compare(b.hash);
        return cmp < 0 || (cmp == 0 && a.n < b.n);
    }

    friend bool operator==(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash == b.hash && a.n == b.n);
    }

    friend bool operator!=(const COutPoint& a, const COutPoint& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class CTxIn
{
public:
    COutPoint prevout;
    CScript scriptSig;
    uint32_t nSequence;
    CScriptWitness scriptWitness; //! Only serialized through CTransaction

    /* Setting nSequence to this value for every input in a transaction
     * disables nLockTime. */
    static const uint32_t SEQUENCE_FINAL = 0xffffffff;

    /* Below flags apply in the context of BIP 68*/
    /* If this flag set, CTxIn::nSequence is NOT interpreted as a
     * relative lock-time. */
    static const uint32_t SEQUENCE_LOCKTIME_DISABLE_FLAG = (1 << 31);

    /* If CTxIn::nSequence encodes a relative lock-time and this flag
     * is set, the relative lock-time has units of 512 seconds,
     * otherwise it specifies blocks with a granularity of 1. */
    static const uint32_t SEQUENCE_LOCKTIME_TYPE_FLAG = (1 << 22);

    /* If CTxIn::nSequence encodes a relative lock-time, this mask is
     * applied to extract that lock-time from the sequence field. */
    static const uint32_t SEQUENCE_LOCKTIME_MASK = 0x0000ffff;

    /* In order to use the same number of bits to encode roughly the
     * same wall-clock duration, and because blocks are naturally
     * limited to occur every 600s on average, the minimum granularity
     * for time-based relative lock-time is fixed at 512 seconds.
     * Converting from CTxIn::nSequence to seconds is performed by
     * multiplying by 512 = 2^9, or equivalently shifting up by
     * 9 bits. */
    static const int SEQUENCE_LOCKTIME_GRANULARITY = 9;

    CTxIn()
    {
        nSequence = SEQUENCE_FINAL;
    }

    explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);
    CTxIn(uint256 hashPrevTx, uint32_t nOut, CScript scriptSigIn=CScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(prevout);
        READWRITE(scriptSig);
        READWRITE(nSequence);
    }

    friend bool operator==(const CTxIn& a, const CTxIn& b)
    {
        return (a.prevout   == b.prevout &&
                a.scriptSig == b.scriptSig &&
                a.nSequence == b.nSequence);
    }

    friend bool operator!=(const CTxIn& a, const CTxIn& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

struct CRoleChangeMode {
    uint64_t fRoleD :1;
    uint64_t fRoleA :1;
    uint64_t fRoleR :1;
    uint64_t fRoleL :1;
    uint64_t fRoleC :1;
    uint64_t fRoleM :1;
    uint64_t nReserved :58;
};

struct CPolicyChangeMode {
    uint64_t fPrmnt :1;
    uint64_t nType  :31;
    uint64_t nParam :32;
};

#include <stdio.h> // FIXME
#include <execinfo.h> // FIXME
#include <iostream> // FIXME
#include <unistd.h> // FIXME

/** A generic output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class CTxOut
{
public:
    union
    {
        CAmount nValue;
        struct CRoleChangeMode nRole;
        struct CPolicyChangeMode nPolicy;
    };

    CScript scriptPubKey;

    enum TxType : uint8_t {
        UNINITIALIZED = 0,
        COIN_TRANSFER = 1,
        ROLE_CHANGE   = 2,
        POLICY_CHANGE = 3,
    } nTxType;

    static const uint64_t NULL_ROLE_RESERVED = 0b0000000000000000000000000000000000000000000000000000000000;
    static const uint64_t NULL_POLICY_PARAM  = 0b00000000000000000000000000000000;

    void Stack(const char* funcname, int lineno) const // FIXME
    {
        return;
        void *array[10];
        size_t size;
        size = backtrace(array, 10);
        fprintf(stderr, "\nCTxOut@%p: %s:%d\n", this, funcname, lineno);
        std::cerr << ToString() << std::endl;
        backtrace_symbols_fd(array, size, STDERR_FILENO);
        fflush(stderr);
    }

    void Check(const char* funcname, int lineno) const // FIXME
    {
        if (nTxType == UNINITIALIZED) {
            char addr[20] = {0};
            snprintf(addr, sizeof addr / sizeof *addr, "%p", this);
            Stack(funcname, lineno);
            throw std::logic_error(std::string(funcname) + ":" + std::to_string(lineno) + "> Invalid nTxType: " + std::to_string(nTxType) + " CTxOut@" + std::string((char*)addr));
        }
    }

    CTxOut()
    {
        nTxType = UNINITIALIZED;
        SetNull();
        Stack(__func__, __LINE__); // FIXME
    }

    CTxOut(const CAmount& nValueIn, CScript scriptPubKeyIn);
    CTxOut(
        const bool fRoleMIn,
        const bool fRoleCIn,
        const bool fRoleLIn,
        const bool fRoleRIn,
        const bool fRoleAIn,
        const bool fRoleDIn,
        CScript scriptPubKeyIn);
    CTxOut(const CRoleChangeMode& nRolesIn, CScript scriptPubKeyIn);
    CTxOut(
        const bool fPermanentIn,
        const uint32_t nTypeIn,
        const uint32_t nParamIn,
        CScript scriptPubKeyIn);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(nValue);
        READWRITE(scriptPubKey);
    }

    void SetNull();
    bool IsNull() const;

    friend bool operator==(const CTxOut& a, const CTxOut& b)
    {
        return (a.nValue       == b.nValue &&
                a.scriptPubKey == b.scriptPubKey &&
                a.nTxType      == b.nTxType);
    }

    friend bool operator!=(const CTxOut& a, const CTxOut& b)
    {
        return !(a == b);
    }

    virtual std::string ToString() const;
};

struct CMutableTransaction;

/**
 * Basic transaction serialization format:
 * - int32_t nVersion
 * - std::vector<CTxIn> vin
 * - std::vector<CTxOut> vout
 * - uint32_t nLockTime
 *
 * Extended transaction serialization format:
 * - int32_t nVersion
 * - unsigned char dummy = 0x00
 * - unsigned char flags (!= 0)
 * - std::vector<CTxIn> vin
 * - std::vector<CTxOut> vout
 * - if (flags & 1):
 *   - CTxWitness wit;
 * - uint32_t nLockTime
 */

template<typename Stream, typename TxType>
inline void UnserializeTransaction(TxType& tx, Stream& s);

template<typename Stream, typename TxType>
inline void SerializeTransaction(const TxType& tx, Stream& s);

/** The basic transaction that is broadcasted on the network and contained in
 * blocks.  A transaction can contain multiple inputs and outputs.
 */
class CTransaction
{
public:
    // Default transaction version.
    static const int32_t CURRENT_VERSION=1944;

    // Changing the default transaction version requires a two step process: first
    // adapting relay policy by bumping MAX_STANDARD_VERSION, and then later date
    // bumping the default CURRENT_VERSION at which point both CURRENT_VERSION and
    // MAX_STANDARD_VERSION will be equal.
    static const int32_t MAX_STANDARD_VERSION=1949;

    // The transaction version determines the type of CTxOut output transaction
    // is stored in the "vout" array of the transaction. "vout" can contain only
    // one type of CTxOut.
    static const int32_t VERSION_COINBASE_TRANSFER = 1944;
    static const int32_t VERSION_COIN_TRANSFER     = 1945;
    static const int32_t VERSION_ROLE_CHANGE       = 1946;
    static const int32_t VERSION_POLICY_CHANGE     = 1947;
    static const int32_t VERSION_ROLE_CHANGE_FEE   = 1948;
    static const int32_t VERSION_POLICY_CHANGE_FEE = 1949;

    // The local variables are made const to prevent unintended modification
    // without updating the cached hash value. However, CTransaction is not
    // actually immutable; deserialization and assignment are implemented,
    // and bypass the constness. This is safe, as they update the entire
    // structure, including the hash.
    const std::vector<CTxIn> vin;
    const std::vector<CTxOut> vout;
    const int32_t nVersion;
    const uint32_t nLockTime;

private:
    /** Memory only. */
    const uint256 hash;

    uint256 ComputeHash() const;

public:
    /** Construct a CTransaction that qualifies as IsNull() */
    CTransaction();

    /** Convert a CMutableTransaction into a CTransaction. */
    CTransaction(const CMutableTransaction &tx);
    CTransaction(CMutableTransaction &&tx);

    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeTransaction(*this, s);
    }

    /** This deserializing constructor is provided instead of an Unserialize method.
     *  Unserialize is not possible, since it would require overwriting const fields. */
    template <typename Stream>
    CTransaction(deserialize_type, Stream& s) : CTransaction(CMutableTransaction(deserialize, s)) {}

    bool IsNull() const {
        return vin.empty() && vout.empty();
    }

    const uint256& GetHash() const {
        return hash;
    }

    // Compute a hash that includes both transaction and witness data
    uint256 GetWitnessHash() const;

    // Return sum of txouts.
    CAmount GetValueOut() const;
    // GetValueIn() is a method on CCoinsViewCache, because
    // inputs must be known to compute value in.

    /**
     * Get the total transaction size in bytes, including witness data.
     * "Total Size" defined in BIP141 and BIP144.
     * @return Total transaction size in bytes
     */
    unsigned int GetTotalSize() const;

    bool IsCoinBase() const
    {
        return (vin.size() == 1 && vin[0].prevout.IsNull());
    }

    friend bool operator==(const CTransaction& a, const CTransaction& b)
    {
        return a.hash == b.hash;
    }

    friend bool operator!=(const CTransaction& a, const CTransaction& b)
    {
        return a.hash != b.hash;
    }

    std::string ToString() const;

    bool HasWitness() const
    {
        for (size_t i = 0; i < vin.size(); i++) {
            if (!vin[i].scriptWitness.IsNull()) {
                return true;
            }
        }
        return false;
    }
};

/** A mutable version of CTransaction. */
struct CMutableTransaction
{
    std::vector<CTxIn> vin;
    std::vector<CTxOut> vout;
    int32_t nVersion;
    uint32_t nLockTime;

    CMutableTransaction();
    CMutableTransaction(const CTransaction& tx);

    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeTransaction(*this, s);
    }


    template <typename Stream>
    inline void Unserialize(Stream& s) {
        UnserializeTransaction(*this, s);
    }

    template <typename Stream>
    CMutableTransaction(deserialize_type, Stream& s) {
        Unserialize(s);
    }

    /** Compute the hash of this CMutableTransaction. This is computed on the
     * fly, as opposed to GetHash() in CTransaction, which uses a cached result.
     */
    uint256 GetHash() const;

    friend bool operator==(const CMutableTransaction& a, const CMutableTransaction& b)
    {
        return a.GetHash() == b.GetHash();
    }

    bool HasWitness() const
    {
        for (size_t i = 0; i < vin.size(); i++) {
            if (!vin[i].scriptWitness.IsNull()) {
                return true;
            }
        }
        return false;
    }
};

typedef std::shared_ptr<const CTransaction> CTransactionRef;
static inline CTransactionRef MakeTransactionRef() { return std::make_shared<const CTransaction>(); }
template <typename Tx> static inline CTransactionRef MakeTransactionRef(Tx&& txIn) { return std::make_shared<const CTransaction>(std::forward<Tx>(txIn)); }

/**
 * Basic transaction serialization format:
 * - int32_t nVersion
 * - std::vector<CTxIn> vin
 * - std::vector<CTxOut> vout
 * - uint32_t nLockTime
 *
 * Extended transaction serialization format:
 * - int32_t nVersion
 * - unsigned char dummy = 0x00
 * - unsigned char flags (!= 0)
 * - std::vector<CTxIn> vin
 * - std::vector<CTxOut> vout
 * - if (flags & 1):
 *   - CTxWitness wit;
 * - uint32_t nLockTime
 */
template<typename Stream, typename TxType>
inline void UnserializeTransaction(TxType& tx, Stream& s) {
    const bool fAllowWitness = !(s.GetVersion() & SERIALIZE_TRANSACTION_NO_WITNESS);

    s >> tx.nVersion;
    unsigned char flags = 0;
    tx.vin.clear();
    tx.vout.clear();
    /* Try to read the vin. In case the dummy is there, this will be read as an empty vector. */
    s >> tx.vin;
    if (tx.vin.size() == 0 && fAllowWitness) {
        /* We read a dummy or an empty vin. */
        s >> flags;
        if (flags != 0) {
            s >> tx.vin;
            s >> tx.vout;
        }
    } else {
        //* We read a non-empty vin. Assume a normal vout follows. */
        s >> tx.vout;
    }

    if (tx.vout.size() > 0) {
        // For all transaction types except coinbase, the first vout is always a role change
        // so that a user is giving himself his own role (to prevent replay attacks).
        if (tx.nVersion == CTransaction::VERSION_COINBASE_TRANSFER) {
            for (size_t i = 0; i < tx.vout.size(); ++i)
                tx.vout[i].nTxType = CTxOut::COIN_TRANSFER;
            // TODO: mark the change as coinbase
        }
        else {
            size_t i = 0;
            tx.vout[i++].nTxType = CTxOut::ROLE_CHANGE;
            // For transactions that carry a fee, the second vout is a change address
            if (tx.vout.size() > 1) {
                switch (tx.nVersion) {
                    case CTransaction::VERSION_COIN_TRANSFER:
                    case CTransaction::VERSION_ROLE_CHANGE_FEE:
                    case CTransaction::VERSION_POLICY_CHANGE_FEE:
                        tx.vout[i++].nTxType = CTxOut::COIN_TRANSFER;
                        break;
                }
            }
            // The following vout entries depend on the tx version
            CTxOut::TxType txtype = CTxOut::UNINITIALIZED;
            switch (tx.nVersion) {
                case CTransaction::VERSION_ROLE_CHANGE:
                case CTransaction::VERSION_ROLE_CHANGE_FEE:
                    txtype = CTxOut::ROLE_CHANGE;
                    break;
                case CTransaction::VERSION_POLICY_CHANGE:
                case CTransaction::VERSION_POLICY_CHANGE_FEE:
                    txtype = CTxOut::POLICY_CHANGE;
                    break;
                case CTransaction::VERSION_COIN_TRANSFER:
                    txtype = CTxOut::COIN_TRANSFER;
                    break;
                default:
                    // Unrecognized version
                    throw std::ios_base::failure(std::string(__func__) + ":" + std::to_string(__LINE__) + "> Unknown transaction version: " + std::to_string(tx.nVersion)); // FIXME
            }
            while (i < tx.vout.size()) {
                tx.vout[i++].nTxType = txtype;
            }
            for (size_t i = 0; i < tx.vout.size(); i++) { // FIXME
                tx.vout[i].Check(__func__, __LINE__);;
            }
        }
    }
    if ((flags & 1) && fAllowWitness) {
        /* The witness flag is present, and we support witnesses. */
        flags ^= 1;
        for (size_t i = 0; i < tx.vin.size(); i++) {
            s >> tx.vin[i].scriptWitness.stack;
        }
    }
    if (flags) {
        /* Unknown flag in the serialization */
        throw std::ios_base::failure("Unknown transaction optional data");
    }
    s >> tx.nLockTime;
}

template<typename Stream, typename TxType>
inline void SerializeTransaction(const TxType& tx, Stream& s) {
    const bool fAllowWitness = !(s.GetVersion() & SERIALIZE_TRANSACTION_NO_WITNESS);

    s << tx.nVersion;
    unsigned char flags = 0;
    // Consistency check
    if (fAllowWitness) {
        /* Check whether witnesses need to be serialized. */
        if (tx.HasWitness()) {
            flags |= 1;
        }
    }
    if (flags) {
        /* Use extended format in case witnesses are to be serialized. */
        std::vector<CTxIn> vinDummy;
        s << vinDummy;
        s << flags;
    }
    s << tx.vin;
    s << tx.vout;
    if (flags & 1) {
        for (size_t i = 0; i < tx.vin.size(); i++) {
            s << tx.vin[i].scriptWitness.stack;
        }
    }
    s << tx.nLockTime;
}


#endif // BITCOIN_PRIMITIVES_TRANSACTION_H

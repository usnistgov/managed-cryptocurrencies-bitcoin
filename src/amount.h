// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_AMOUNT_H
#define BITCOIN_AMOUNT_H

#include <stdint.h>
#include <serialize.h>

/** Amount in satoshis (Can be negative) */
typedef int64_t CAmountType;

class CAmount
{
public:
	enum CTXMode {
		MODE_COIN_TRANSFER = 0b00,
		MODE_ROLE_CHANGE   = 0b10,
		MODE_POLICY_CHANGE = 0b11,
	};

	struct CRoleChangeMode {
		uint64_t reserved :56;
		uint64_t action :1;
		uint64_t a_role :1;
		uint64_t u_role :1;
		uint64_t l_role :1;
		uint64_t c_role :1;
		uint64_t m_role :1;
		uint64_t mode   :2;
	};

	struct CPolicyChangeMode {
		uint64_t param :32;
		uint64_t type  :29;
		uint64_t perm  :1;
		uint64_t mode  :2;
	};

	struct CCoinTransferMode {
		CAmountType amount :63;
		CAmountType mode   :1;
	};

	union {
		struct CRoleChangeMode   rcm;
		struct CPolicyChangeMode pcm;
		struct CCoinTransferMode ctm;
		CAmountType val;
	} nValue;

	CAmount() {
		this->nValue.ctm.mode  = 0;
		this->nValue.ctm.amount = 0;
	}
	CAmount(const CAmountType& nValue) {
		this->nValue.ctm.mode  = 0;
		this->nValue.ctm.amount = nValue;
	}
	CAmount(const bool action, const bool m_role, const bool c_role, const bool l_role, const bool u_role, const bool a_role) {
		this->nValue.rcm.reserved = 0;
		this->nValue.rcm.action = action;
		this->nValue.rcm.a_role = a_role;
		this->nValue.rcm.u_role = u_role;
		this->nValue.rcm.l_role = l_role;
		this->nValue.rcm.c_role = c_role;
		this->nValue.rcm.m_role = m_role;
		this->nValue.rcm.mode = MODE_ROLE_CHANGE;
	}

	CTXMode GetTXMode() const {
		if (this->nValue.ctm.mode == MODE_COIN_TRANSFER)
			return MODE_COIN_TRANSFER;
		if (this->nValue.rcm.mode == MODE_ROLE_CHANGE)
			return MODE_ROLE_CHANGE;
		return MODE_POLICY_CHANGE;
	}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	void SerializationOp(Stream& s, Operation ser_action) {
		READWRITE(nValue.val);
	}

	CAmount& operator=(const CAmount& other) {
		if (this != &other)
			this->nValue = other.nValue;
	        return *this;
	}
	CAmount& operator=(const CAmountType& other) {
		this->nValue.ctm.amount = other;
	        return *this;
	}

	CAmount operator-() const {
		return CAmount(-(this->nValue.ctm.amount));
	}
	CAmountType* operator&() {
		return &this->nValue.val;
	}
	operator CAmountType() const {
		return this->nValue.ctm.amount;
	}

	CAmount& operator+=(const CAmountType& val) {
		this->nValue.ctm.amount += val;
		return *this;
	}
	CAmount& operator-=(const CAmountType& val) {
		this->nValue.ctm.amount -= val;
		return *this;
	}
	CAmount& operator*=(const CAmountType& val) {
		this->nValue.ctm.amount *= val;
		return *this;
	}
	CAmount& operator/=(const CAmountType& val) {
		this->nValue.ctm.amount /= val;
		return *this;
	}
	CAmount operator/(const CAmountType& val) const {
		return CAmount(this->nValue.ctm.amount / val);
	}
	CAmount operator/(const int& val) const {
		return CAmount(this->nValue.ctm.amount / val);
	}
	CAmount operator/(const unsigned int& val) const {
		return CAmount(this->nValue.ctm.amount / val);
	}
	CAmount& operator>>=(const int& val) {
		this->nValue.ctm.amount >>= val;
		return *this;
	}
	CAmount& operator<<=(const int& val) {
		this->nValue.ctm.amount <<= val;
		return *this;
	}
};

static const CAmount COIN = 100000000;
static const CAmount CENT = 1000000;

/** No amount larger than this (in satoshi) is valid.
 *
 * Note that this constant is *not* the total money supply, which in Bitcoin
 * currently happens to be less than 21,000,000 BTC for various reasons, but
 * rather a sanity check. As this sanity check is used by consensus-critical
 * validation code, the exact value of the MAX_MONEY constant is consensus
 * critical; in unusual circumstances like a(nother) overflow bug that allowed
 * for the creation of coins out of thin air modification could lead to a fork.
 * */
static const CAmount MAX_MONEY = 21000000 * COIN;
inline bool MoneyRange(const CAmount& nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

#endif //  BITCOIN_AMOUNT_H

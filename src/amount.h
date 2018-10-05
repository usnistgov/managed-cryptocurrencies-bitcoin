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
private:
	CAmountType nValue;

public:
	CAmount() {
		this->nValue = 0;
	}
	CAmount(const CAmountType& nValue) {
		this->nValue = nValue;
	}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	void SerializationOp(Stream& s, Operation ser_action) {
		READWRITE(nValue);
	}

	CAmount& operator=(const CAmount& other) {
		if (this != &other)
			this->nValue = other.nValue;
	        return *this;
	}
	CAmount& operator=(const CAmountType& other) {
		this->nValue = other;
	        return *this;
	}

	CAmount operator-() const {
		return CAmount(-(this->nValue));
	}
	CAmountType* operator&() {
		return &this->nValue;
	}
	operator CAmountType() const {
		return this->nValue;
	}

	CAmount& operator+=(const CAmountType& val) {
		this->nValue += val;
		return *this;
	}
	CAmount& operator-=(const CAmountType& val) {
		this->nValue -= val;
		return *this;
	}
	CAmount& operator*=(const CAmountType& val) {
		this->nValue *= val;
		return *this;
	}
	CAmount& operator/=(const CAmountType& val) {
		this->nValue /= val;
		return *this;
	}
	CAmount operator/(const CAmountType& val) const {
		return CAmount(this->nValue / val);
	}
	CAmount operator/(const int& val) const {
		return CAmount(this->nValue / val);
	}
	CAmount& operator>>=(const int& val) {
		this->nValue >>= val;
		return *this;
	}
	CAmount& operator<<=(const int& val) {
		this->nValue <<= val;
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

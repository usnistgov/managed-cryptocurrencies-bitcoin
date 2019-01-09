// Copyright (c) 2018-2019 National Institute of Standards and Technology
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_ACCOUNT_DB_H
#define BITCOIN_ACCOUNT_DB_H

#include <accounts/data.h>
#include <sys/stat.h>
#include <util.h>


class CManagedAccountDB {
public:
    CManagedAccountDB() {
        InitDB();
    }

    CManagedAccountDB(std::string filePath) {
        dbFilePath = filePath;
        InitDB();
    }

    void ResetDB();
    bool AddAccount(CTxDestination address, CManagedAccountData account);
    bool UpdateAccount(CTxDestination address, CManagedAccountData account);
    bool DeleteAccount(CTxDestination address);
    CTxDestination GetRootAddress();
    bool GetAccountByAddress(CTxDestination address, CManagedAccountData& account);
    bool ExistsAccountForAddress(CTxDestination address);
    int size();
private:
    void InitDB();

    // Serialization methods
    void SaveToDisk();
    void LoadFromDisk();

    // Class attributes
    std::map <CTxDestination, CManagedAccountData> accountDB;
    CTxDestination rootAccountAddress;
    std::string dbFilePath = GetDataDir().string() + "/accounts.dat";
};

#endif // BITCOIN_ACCOUNT_DB_H

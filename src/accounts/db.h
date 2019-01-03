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
        // Check if file exists at initialization
        struct stat buffer;
        if(stat (dbFilePath.c_str(), &buffer) == 0) {
            std::cout << __func__ << ":" << __LINE__ << "> Loading DB from disk" << std::endl;  // FIXME
            LoadFromDisk();
        } else {  // File does not exist, a new map is initialized
            std::cout << __func__ << ":" << __LINE__ << "> Init empty DB" << std::endl;  // FIXME
            accountDB = std::map<CTxDestination, CManagedAccountData>();
        }
    }

    bool AddAccount(CTxDestination address, CManagedAccountData account);

    bool UpdateAccount(CTxDestination address, CManagedAccountData account);

    bool DeleteAccount(CTxDestination address);

    bool GetAccountByAddress(CTxDestination address);

    bool ExistsAccountForAddress(CTxDestination address);


private:
    // Serialization methods
    void SaveToDisk();
    void LoadFromDisk();

    // Class attributes
    std::map <CTxDestination, CManagedAccountData> accountDB;
    const std::string dbFilePath = GetDataDir().string() + "/accounts.dat";
};

#endif // BITCOIN_ACCOUNT_DB_H

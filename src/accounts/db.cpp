// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <accounts/db.h>

#include <iostream>
#include <fstream>

bool CManagedAccountDB::AddAccount(CTxDestination address, CManagedAccountData account) {
    accountDB.insert(std::make_pair(address,account));

    SaveToDisk();
    return true;
}

bool CManagedAccountDB::UpdateAccount(CTxDestination address, CManagedAccountData account) {
    // FIXME improve logging
    std::cout << __func__ << ":" << __LINE__ << "> Updating node: " << EncodeDestination(address) << std::endl;
    std::cout << __func__ << ":" << __LINE__ << ">   Parent: " ;
    std::cout << (IsValidDestination(account.GetParent())? EncodeDestination(account.GetParent()) : "-none-");
    std::cout << std::endl;
    std::cout << __func__ << ":" << __LINE__ << ">    Roles: ";
    std::cout << ValueFromRoles(account.GetRoles()).get_str() << std::endl;
    std::cout << __func__ << ":" << __LINE__ << "> ~~~~~~ " << std::endl;

    if(accountDB.find(address) == accountDB.end()) {
        return AddAccount(address, account);
    } else {
        SaveToDisk();
        return true;
    }
}

bool CManagedAccountDB::DeleteAccount(CTxDestination address) {

    SaveToDisk();
    return false;
}

bool CManagedAccountDB::GetAccountByAddress(CTxDestination address) {
    return false;
}

bool CManagedAccountDB::ExistsAccountForAddress(CTxDestination address) {
    return false;
}

// Serialization methods
void CManagedAccountDB::SaveToDisk() {
    std::ofstream file;
    file.open(dbFilePath.c_str());

    std::map<CTxDestination, CManagedAccountData>::iterator iter;

    for(iter=accountDB.begin(); iter!=accountDB.end(); ++iter)
    {
        std::cout << __func__ << ":" << __LINE__ << "> Saving " << EncodeDestination(iter->first) << std::endl;  // FIXME
        file << EncodeDestination(iter->first) << std::endl;
        file << iter->second << std::endl;
    }

    file.close();
}

void CManagedAccountDB::LoadFromDisk() {
    std::ifstream file;
    std::string line;
    bool isKey = true;
    CTxDestination accountAddres;
    CManagedAccountData accountData;

    file.open(dbFilePath.c_str());

    while(file >> line >> accountData)
    {
        accountDB.insert(std::make_pair(DecodeDestination(line), accountData));
    }

    file.close();
    std::cout << __func__ << ":" << __LINE__ << "> " << accountDB.size() << " account(s) loaded" << std::endl;  // FIXME
}

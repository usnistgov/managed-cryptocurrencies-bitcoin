// Copyright (c) 2018-2019 National Institute of Standards and Technology
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <accounts/db.h>

#include <iostream>
#include <fstream>

bool CManagedAccountDB::AddAccount(CTxDestination address, CManagedAccountData account) {
    std::cout << __func__ << ":" << __LINE__ << "> Adding node: " << EncodeDestination(address) << " -> ";  // FIXME
    std::cout << account.ToString() << std::endl;  // FIXME
    std::cout << __func__ << ":" << __LINE__ << "> ~~~~~~ " << std::endl;  // FIXME

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
    std::cout << __func__ << ":" << __LINE__ << "> Children: ";

    for(const auto& child:account.GetChildren()) {
        std::cout << EncodeDestination(child) << " ";
    }
    std::cout << std::endl;
    std::cout << __func__ << ":" << __LINE__ << "> ~~~~~~ " << std::endl;

    if(!ExistsAccountForAddress(address)) {
        return AddAccount(address, account);
    } else {
        accountDB.at(address) = account;
        SaveToDisk();
        return true;
    }
}

bool CManagedAccountDB::DeleteAccount(CTxDestination address) {
    auto accountIter = accountDB.find(address);

    if(accountIter == accountDB.end()) {
        return false;
    } else {
        accountDB.erase(accountIter);
        SaveToDisk();
        return true;
    }
}

bool CManagedAccountDB::GetAccountByAddress(CTxDestination address, CManagedAccountData& account) {
    if(ExistsAccountForAddress(address)) {
        account = accountDB.at(address);
        return true;
    }

    return false;
}

bool CManagedAccountDB::ExistsAccountForAddress(CTxDestination address) {
    return (accountDB.find(address) != accountDB.end());
}


int CManagedAccountDB::size() {
    return accountDB.size();
}

void CManagedAccountDB::ResetDB() {
    accountDB = std::map<CTxDestination, CManagedAccountData>();
    SaveToDisk();
}

void CManagedAccountDB::InitDB() {
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

// Serialization methods
void CManagedAccountDB::SaveToDisk() {
    std::ofstream file;
    file.open(dbFilePath.c_str());

    std::map<CTxDestination, CManagedAccountData>::iterator iter;

    for(iter=accountDB.begin(); iter!=accountDB.end(); ++iter)
    {
        std::cout << __func__ << ":" << __LINE__ << "> Saving " << EncodeDestination(iter->first) << " == ";  // FIXME
        std::cout << (iter->second).ToString() << std::endl;  // FIXME
        file << EncodeDestination(iter->first) << std::endl;
        file << iter->second << std::endl;
    }

    file.close();
}

void CManagedAccountDB::LoadFromDisk() {
    std::ifstream file;
    std::string address;
    CManagedAccountData accountData;

    file.open(dbFilePath.c_str());

    while(file >> address >> accountData)
    {
        accountDB.insert(std::make_pair(DecodeDestination(address), accountData));
        accountData.~CManagedAccountData();
        new (&accountData) CManagedAccountData();
    }

    file.close();
    std::cout << __func__ << ":" << __LINE__ << "> " << accountDB.size() << " account(s) loaded" << std::endl;  // FIXME
}

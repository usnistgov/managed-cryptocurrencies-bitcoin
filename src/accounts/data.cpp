// Copyright (c) 2018-2019 National Institute of Standards and Technology
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <accounts/data.h>

bool CManagedAccountData::AddChild(CTxDestination child)
{
    if(std::find(accountChildren.begin(), accountChildren.end(), child) == accountChildren.end()) {
        accountChildren.push_back(child);
        return true;
    } else {
        return false;
    }
}

bool CManagedAccountData::RemoveChild(CTxDestination child)
{
    if(std::find(accountChildren.begin(), accountChildren.end(), child) == accountChildren.end()) {
        return false;
    } else {
        std::remove(accountChildren.begin(), accountChildren.end(), child);
        return true;
    }
}

CTxDestination CManagedAccountData::GetParent()
{
    return accountParent;
}

std::vector<CTxDestination> CManagedAccountData::GetChildren()
{
    return accountChildren;
}

CRoleChangeMode CManagedAccountData::GetRoles()
{
    return accountRoles;
}

std::string CManagedAccountData::ToString() {
    std::string output = ValueFromRoles(GetRoles()).get_str() + ";" + EncodeDestination(GetParent()) + ";";

    for(int i=0; i<accountChildren.size(); i++) {
        output += EncodeDestination(accountChildren.at(i));

        if(i != accountChildren.size() - 1) {
            output += "|";
        }
    }

    return output;
}

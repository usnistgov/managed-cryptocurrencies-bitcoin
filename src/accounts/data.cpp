// Copyright (c) 2018-2019 National Institute of Standards and Technology
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <accounts/data.h>

bool CManagedAccountData::AddChild(const CTxDestination& child)
{
    if(std::find(accountChildren.begin(), accountChildren.end(), child) == accountChildren.end()) {
        accountChildren.push_back(child);
        return true;
    }
    return false;
}

bool CManagedAccountData::RemoveChild(const CTxDestination& child)
{
    auto childIter = std::find(accountChildren.begin(), accountChildren.end(), child);

    if(childIter != accountChildren.end()) {
        accountChildren.erase(childIter);
        return true;
    }
    return false;
}

const CTxDestination& CManagedAccountData::GetParent() const
{
    return accountParent;
}

void CManagedAccountData::SetParent(const CTxDestination& parentAddress)
{
    accountParent = parentAddress;
}

const std::vector<CTxDestination>& CManagedAccountData::GetChildren() const
{
    return accountChildren;
}

const CRoleChangeMode& CManagedAccountData::GetRoles() const
{
    return accountRoles;
}

void CManagedAccountData::SetRoles(const CRoleChangeMode& inputRoles)
{
    accountRoles = inputRoles;
}

std::string CManagedAccountData::ToString() const
{
    std::string output = ValueFromRoles(GetRoles()).get_str() + " | " + EncodeDestination(GetParent()) + " | ";

    for(size_t i=0; i<accountChildren.size(); i++) {
        output += EncodeDestination(accountChildren.at(i));

        if(i != accountChildren.size() - 1) {
            output += " , ";
        }
    }

    return output;
}

// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <accounts/data.h>

bool CManagedAccountData::AddChildren(CTxDestination child)
{
    return false;
}

CTxDestination CManagedAccountData::GetParent()
{
    return accountParent;
}

std::vector<CTxDestination> CManagedAccountData::GetChildren()
{
    return {};
}

CRoleChangeMode CManagedAccountData::GetRoles()
{
    return accountRoles;
}

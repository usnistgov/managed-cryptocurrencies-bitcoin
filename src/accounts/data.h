// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_ACCOUNT_H
#define BITCOIN_ACCOUNT_H

#include <core_io.h>
#include <univalue.h>
#include <utilmoneystr.h>
#include <script/standard.h>
#include <key.h>
#include <stdint.h>
#include <base58.h>

#include <string>
#include <boost/algorithm/string.hpp>


class CManagedAccountData {
public:
    CManagedAccountData() {}

    CManagedAccountData(CRoleChangeMode roles)
    {
        accountRoles = roles;
    }

    CManagedAccountData(CRoleChangeMode roles, CTxDestination parent)
    {
        accountRoles = roles;
        accountParent = parent;
    }

    bool AddChildren(CTxDestination child);
    CRoleChangeMode GetRoles();
    CTxDestination GetParent();
    std::vector <CTxDestination> GetChildren();


    friend std::ostream & operator << (std::ostream &out, const CManagedAccountData &obj)
    {
        out << ValueFromRoles(obj.GetRoles()).get_str() << ";" << EncodeDestination(obj.GetParent()) << ";";

        for(int i=0; i<obj.GetChildren().size(); i++) {
            out << EncodeDestination(obj.GetChildren().at(i));

            if(i != obj.GetChildren().size()-1) {
                out << "|";
            }
        }

        return out;
    }

    friend std::istream & operator >> (std::istream &in,  CManagedAccountData &obj)
    {
        std::string accountObjRaw;
        std::vector<std::string> accountData;

        in >> accountObjRaw;

        // If there is nothing to parse, exit the function
        if(accountObjRaw=="") {
            return in;
        }

        boost::split(accountData, accountObjRaw, [](char c){return c == ';';});

        ParseRoles(accountData[0], obj.accountRoles);
        obj.accountParent = DecodeDestination(accountData[1]);

        return in;
    }
private:
    CRoleChangeMode accountRoles;
    CTxDestination accountParent;
    std::vector <CTxDestination> accountChildren;
};

#endif // BITCOIN_ACCOUNT_H

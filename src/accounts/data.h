// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_ACCOUNT_H
#define BITCOIN_ACCOUNT_H

#include <script/standard.h>
#include <key.h>
#include <stdint.h>
#include <base58.h>

#include <string>
#include <boost/algorithm/string.hpp>


class CManagedAccountData {
public:
    CManagedAccountData() {}

    CManagedAccountData(int roles)
    {
        accountRoles = roles;
    }

    CManagedAccountData(int roles, CTxDestination parent)
    {
        accountRoles = roles;
        accountParent = parent;
    }

    bool AddChildren(CTxDestination child);
    int GetRoles();
    CTxDestination GetParent();
    std::vector <CTxDestination> GetChildren();


    friend std::ostream & operator << (std::ostream &out, const CManagedAccountData &obj)
    {
        out << std::to_string(obj.GetRoles()) << ";" << EncodeDestination(obj.GetParent()) << ";";

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

        obj.accountRoles = std::stoi(accountData[0]);
        obj.accountParent = DecodeDestination(accountData[1]);

        return in;
    }
private:
    int accountRoles;
    CTxDestination accountParent;
    std::vector <CTxDestination> accountChildren;
};

#endif // BITCOIN_ACCOUNT_H

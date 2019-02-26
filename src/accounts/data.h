// Copyright (c) 2018-2019 National Institute of Standards and Technology
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

    CManagedAccountData(const CRoleChangeMode& roles, const CTxDestination& parent)
    {
        accountRoles = roles;
        accountParent = parent;
    }

    bool AddChild(const CTxDestination& child);
    bool RemoveChild(const CTxDestination& child);
    const CRoleChangeMode& GetRoles() const;
    void SetRoles(const CRoleChangeMode& inputRoles);
    const CTxDestination& GetParent() const;
    const std::vector <CTxDestination>& GetChildren() const;

    std::string ToString() const;

    // Serialization methods
    friend std::ostream & operator << (std::ostream &out, const CManagedAccountData &obj)
    {
        out << ValueFromRoles(obj.GetRoles()).get_str() << ";" << EncodeDestination(obj.GetParent()) << ";";

        for(size_t i=0; i<obj.GetChildren().size(); i++) {
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

        ParseRoles(accountData.at(0), obj.accountRoles);
        obj.accountParent = DecodeDestination(accountData.at(1));

        if(accountData.at(2) != "") {
            std::vector<std::string> accountChildrenRaw;
            boost::split(accountChildrenRaw, accountData.at(2), [](char c){return c == '|';});

            for(size_t i=0; i<accountChildrenRaw.size(); i++) {
                obj.AddChild(DecodeDestination(accountChildrenRaw.at(i)));
            }
        }

        return in;
    }
private:
    CRoleChangeMode accountRoles;
    CTxDestination accountParent;
    std::vector <CTxDestination> accountChildren;
};

#endif // BITCOIN_ACCOUNT_H

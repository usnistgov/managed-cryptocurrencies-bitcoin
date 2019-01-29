#include <test/test_bitcoin.h>
#include <boost/test/unit_test.hpp>

#include <accounts/data.h>
#include <accounts/db.h>

BOOST_FIXTURE_TEST_SUITE(accounts_tests, BasicTestingSetup)


BOOST_AUTO_TEST_CASE(account_data_tests)
{
    std::vector<std::string> sampleAddresses = {
        "1ArmQouzU8cvAt4muQJ9srPy7CXVcgbSmU",
        "1NWqvweBVX1D5C1E9h5vbdX85L7TsDAsgu"
    };
    bool status;

    // Check default constructor
    CManagedAccountData accountDataA;

    BOOST_CHECK(
        ValueFromRoles(accountDataA.GetRoles()).get_str() == "......"
    );
    BOOST_CHECK(
        accountDataA.GetChildren().size() == 0
    );
    BOOST_CHECK(
        !IsValidDestination(accountDataA.GetParent())
    );

    // Check constructor with roles
    std::string strRolesB = "M..R..";
    CRoleChangeMode rolesB;
    ParseRoles(strRolesB, rolesB);
    CManagedAccountData accountDataB(rolesB);

    BOOST_CHECK(
        ValueFromRoles(accountDataB.GetRoles()).get_str() == strRolesB
    );
    BOOST_CHECK(
        accountDataB.GetChildren().size() == 0
    );
    BOOST_CHECK(
        !IsValidDestination(accountDataB.GetParent())
    );

    // Check constructor with roles and parent
    std::string strRolesC = "..LR..";
    CRoleChangeMode rolesC;
    ParseRoles(strRolesC, rolesC);
    CTxDestination parentC = DecodeDestination(sampleAddresses.at(0));
    CManagedAccountData accountDataC(rolesC, parentC);

    std::cout << "RolesC == " << ValueFromRoles(accountDataC.GetRoles()).get_str() << std::endl;
    std::cout << "RolesC == " << ValueFromRoles(rolesC).get_str() << std::endl;
    std::cout << "ParentC == " << EncodeDestination(parentC) << std::endl;

    BOOST_CHECK(
        ValueFromRoles(accountDataC.GetRoles()).get_str() == strRolesC
    );
    BOOST_CHECK(
        accountDataC.GetChildren().size() == 0
    );
    BOOST_CHECK(
        IsValidDestination(accountDataC.GetParent())
    );

    status = accountDataC.AddChild(DecodeDestination(sampleAddresses.at(1)));

    BOOST_CHECK(
        accountDataC.GetChildren().size() == 1
    );
    BOOST_CHECK(
        status
    );

    status = accountDataC.RemoveChild(DecodeDestination(sampleAddresses.at(1)));

    BOOST_CHECK(
        accountDataC.GetChildren().size() == 0
    );
    BOOST_CHECK(
        status
    );
}

BOOST_AUTO_TEST_CASE(account_db_tests)
{
    std::vector<std::string> sampleAddresses = {
        "1ArmQouzU8cvAt4muQJ9srPy7CXVcgbSmU",
        "1NWqvweBVX1D5C1E9h5vbdX85L7TsDAsgu"
    };
    bool status;
    CManagedAccountData sampleAccountData;

    CManagedAccountDB accountDB("/tmp/accounts.dat");
    accountDB.ResetDB();

    status = accountDB.AddAccount(DecodeDestination(sampleAddresses.at(0)), sampleAccountData);

    BOOST_CHECK(
        accountDB.size() == 1
    );
    BOOST_CHECK(
        status
    );

    status = accountDB.UpdateAccount(DecodeDestination(sampleAddresses.at(1)), sampleAccountData);

    BOOST_CHECK(
        accountDB.size() == 2
    );
    BOOST_CHECK(
        status
    );

    status = accountDB.UpdateAccount(DecodeDestination(sampleAddresses.at(1)), sampleAccountData);

    BOOST_CHECK(
        accountDB.size() == 2
    );
    BOOST_CHECK(
        status
    );

    status = accountDB.DeleteAccount(DecodeDestination(sampleAddresses.at(0)));

    BOOST_CHECK(
        accountDB.size() == 1
    );

    BOOST_CHECK(
        status
    );

    CManagedAccountData accountDataTest;
    status = accountDB.GetAccountByAddress(DecodeDestination(sampleAddresses.at(0)), accountDataTest);

    BOOST_CHECK(
        !status
    );

    status = accountDB.GetAccountByAddress(DecodeDestination(sampleAddresses.at(1)), accountDataTest);

    BOOST_CHECK(
        ValueFromRoles(accountDataTest.GetRoles()).get_str() == ValueFromRoles(sampleAccountData.GetRoles()).get_str()
    );
    BOOST_CHECK(
        accountDataTest.GetChildren().size() == sampleAccountData.GetChildren().size()
    );
    BOOST_CHECK(
        EncodeDestination(accountDataTest.GetParent()) == EncodeDestination(sampleAccountData.GetParent())
    );
    BOOST_CHECK(
        status
    );

    status = accountDB.ExistsAccountForAddress(DecodeDestination(sampleAddresses.at(0)));

    BOOST_CHECK(
        !status
    );

    status = accountDB.ExistsAccountForAddress(DecodeDestination(sampleAddresses.at(1)));

    BOOST_CHECK(
        status
    );
}

BOOST_AUTO_TEST_CASE(account_db_serialize_tests)
{
    std::vector<std::string> sampleAddresses = {
        "1ArmQouzU8cvAt4muQJ9srPy7CXVcgbSmU",
        "1NWqvweBVX1D5C1E9h5vbdX85L7TsDAsgu",
        "16tgXnXyw7rk2jvDLhuj2kkCJu5my5pwPs",
        "16bzmWkCPBVYBDkaKD6LHsckVnE2qkHHzy"
    };
    bool status;
    std::string strRoles = "......";
    CRoleChangeMode roles1;
    CRoleChangeMode roles2;
    CRoleChangeMode roles3;
    CRoleChangeMode roles4;
    ParseRoles(strRoles, roles1);
    ParseRoles(strRoles, roles2);
    ParseRoles(strRoles, roles3);
    ParseRoles(strRoles, roles4);
    CManagedAccountData sampleAccountData0;
    CManagedAccountData sampleAccountData1(roles1);
    CManagedAccountData sampleAccountData2(roles2, DecodeDestination(sampleAddresses.at(0)));
    CManagedAccountData sampleAccountData3(roles3, DecodeDestination(sampleAddresses.at(0)));
    CManagedAccountData sampleAccountData4(roles4, DecodeDestination(sampleAddresses.at(0)));

    CManagedAccountDB accountDB("/tmp/accounts.dat");
    accountDB.ResetDB();

    status = accountDB.AddAccount(DecodeDestination(sampleAddresses.at(0)), sampleAccountData1);
    BOOST_CHECK(
        accountDB.size() == 1
    );
    BOOST_CHECK(
        status
    );

    accountDB.~CManagedAccountDB();
    new (&accountDB) CManagedAccountDB("/tmp/accounts.dat");

    status = accountDB.GetAccountByAddress(DecodeDestination(sampleAddresses.at(0)), sampleAccountData0);
    BOOST_CHECK(
        status
    );
    BOOST_CHECK(
        sampleAccountData0.GetChildren().size() == 0
    );

    status = accountDB.AddAccount(DecodeDestination(sampleAddresses.at(1)), sampleAccountData2);
    BOOST_CHECK(
        accountDB.size() == 2
    );
    BOOST_CHECK(
        status
    );

    sampleAccountData1.AddChild(DecodeDestination(sampleAddresses.at(1)));
    status = accountDB.UpdateAccount(DecodeDestination(sampleAddresses.at(0)), sampleAccountData1);
    BOOST_CHECK(
        status
    );

    accountDB.~CManagedAccountDB();
    new (&accountDB) CManagedAccountDB("/tmp/accounts.dat");

    status = accountDB.GetAccountByAddress(DecodeDestination(sampleAddresses.at(0)), sampleAccountData0);
    BOOST_CHECK(
        status
    );
    BOOST_CHECK(
        sampleAccountData0.GetChildren().size() == 1
    );
    status = accountDB.GetAccountByAddress(DecodeDestination(sampleAddresses.at(1)), sampleAccountData0);
    BOOST_CHECK(
        status
    );
    BOOST_CHECK(
        sampleAccountData0.GetChildren().size() == 0
    );

    status = accountDB.AddAccount(DecodeDestination(sampleAddresses.at(2)), sampleAccountData3);
    BOOST_CHECK(
        accountDB.size() == 3
    );
    BOOST_CHECK(
        status
    );

    sampleAccountData1.AddChild(DecodeDestination(sampleAddresses.at(2)));
    status = accountDB.UpdateAccount(DecodeDestination(sampleAddresses.at(0)), sampleAccountData1);
    BOOST_CHECK(
        status
    );

    accountDB.~CManagedAccountDB();
    new (&accountDB) CManagedAccountDB("/tmp/accounts.dat");

    status = accountDB.GetAccountByAddress(DecodeDestination(sampleAddresses.at(0)), sampleAccountData0);
    BOOST_CHECK(
        status
    );
    BOOST_CHECK(
        sampleAccountData0.GetChildren().size() == 2
    );

    status = accountDB.GetAccountByAddress(DecodeDestination(sampleAddresses.at(1)), sampleAccountData0);
    BOOST_CHECK(
        status
    );
    BOOST_CHECK(
        sampleAccountData0.GetChildren().size() == 0
    );
    status = accountDB.GetAccountByAddress(DecodeDestination(sampleAddresses.at(2)), sampleAccountData0);
    BOOST_CHECK(
        status
    );
    BOOST_CHECK(
        sampleAccountData0.GetChildren().size() == 0
    );

    status = accountDB.AddAccount(DecodeDestination(sampleAddresses.at(3)), sampleAccountData4);
    BOOST_CHECK(
        accountDB.size() == 4
    );
    BOOST_CHECK(
        status
    );

    sampleAccountData1.AddChild(DecodeDestination(sampleAddresses.at(3)));
    status = accountDB.UpdateAccount(DecodeDestination(sampleAddresses.at(0)), sampleAccountData1);
    BOOST_CHECK(
        status
    );

    accountDB.~CManagedAccountDB();
    new (&accountDB) CManagedAccountDB("/tmp/accounts.dat");

    status = accountDB.GetAccountByAddress(DecodeDestination(sampleAddresses.at(0)), sampleAccountData0);
    BOOST_CHECK(
        status
    );
    BOOST_CHECK(
        sampleAccountData0.GetChildren().size() == 3
    );
    status = accountDB.GetAccountByAddress(DecodeDestination(sampleAddresses.at(1)), sampleAccountData0);
    BOOST_CHECK(
        status
    );
    BOOST_CHECK(
        sampleAccountData0.GetChildren().size() == 0
    );
    status = accountDB.GetAccountByAddress(DecodeDestination(sampleAddresses.at(2)), sampleAccountData0);
    BOOST_CHECK(
        status
    );
    BOOST_CHECK(
        sampleAccountData0.GetChildren().size() == 0
    );
    status = accountDB.GetAccountByAddress(DecodeDestination(sampleAddresses.at(3)), sampleAccountData0);
    BOOST_CHECK(
        status
    );
    BOOST_CHECK(
        sampleAccountData0.GetChildren().size() == 0
    );
}

BOOST_AUTO_TEST_SUITE_END()

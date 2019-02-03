#include <test/test_bitcoin.h>
#include <boost/test/unit_test.hpp>

#include <accounts/visualization.h>
#include <accounts/data.h>
#include <accounts/db.h>

BOOST_FIXTURE_TEST_SUITE(account_visualization_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(graph_tests)
{
    // TEST SETUP
    std::vector<std::string> sampleAddresses = {
        "1ArmQouzU8cvAt4muQJ9srPy7CXVcgbSmU",
        "1NWqvweBVX1D5C1E9h5vbdX85L7TsDAsgu",
        "16tgXnXyw7rk2jvDLhuj2kkCJu5my5pwPs",
        "16bzmWkCPBVYBDkaKD6LHsckVnE2qkHHzy"
    };
    bool status = false;

    CManagedAccountDB accountDB("/tmp/accounts.dat");
    accountDB.ResetDB();

    BOOST_CHECK(
        accountDB.size() == 0
    );

    // destinations
    CTxDestination addressRoot = DecodeDestination(sampleAddresses.at(0));
    CTxDestination address2 = DecodeDestination(sampleAddresses.at(1));
    CTxDestination address3 = DecodeDestination(sampleAddresses.at(2));
    CTxDestination address5 = DecodeDestination(sampleAddresses.at(3));
    
    std::string strRolesRoot = "M..R..";
    CRoleChangeMode rolesRoot;
    ParseRoles(strRolesRoot, rolesRoot);
    CManagedAccountData accountDataRoot(rolesRoot);
    accountDataRoot.AddChild(address2);
    accountDataRoot.AddChild(address3);

    std::string strRoles2 = "M..R..";
    CRoleChangeMode roles2;
    ParseRoles(strRoles2, roles2);
    CManagedAccountData accountData2(roles2,addressRoot);
    accountData2.AddChild(address5);

    std::string strRoles5 = ".C.R..";
    CRoleChangeMode roles5;
    ParseRoles(strRoles5, roles5);
    CManagedAccountData accountData5(roles5,address2);

    std::string strRoles3 = "...RA.";
    CRoleChangeMode roles3;
    ParseRoles(strRoles3, roles3);
    CManagedAccountData accountData3(roles3,addressRoot);

    status = accountDB.AddAccount(addressRoot, accountDataRoot);
    status = accountDB.AddAccount(address2, accountData2);
    status = accountDB.AddAccount(address3, accountData3);
    status = accountDB.AddAccount(address5, accountData5);

    BOOST_CHECK(
        accountDB.size() == 4
    );
    BOOST_CHECK(
        status
    );

    // ACTUAL TEST
    CAccountDataVisualization visu(accountDB);
    visu.VisualizeGraph();

}

BOOST_AUTO_TEST_SUITE_END()

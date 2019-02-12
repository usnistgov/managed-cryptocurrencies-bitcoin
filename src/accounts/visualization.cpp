// Copyright (c) 2018-2019 National Institute of Standards and Technology
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <accounts/visualization.h>

bool CAccountDataVisualization::LoadGraph(){
    CManagedAccountData rootAccount;
    CTxDestination rootAddress = db.GetRootAddress();
    db.GetAccountByAddress(rootAddress, rootAccount);

    std::cout << "loading graph with the following list of accounts: " << std::endl;
    std::cout << db.ToString();
    std::cout << std::flush;

    Vertex rootNode = boost::add_vertex(VertexProperties{EncodeDestination(rootAddress), ValueFromRoles(rootAccount.GetRoles()).get_str()}, g);

    // for(const auto& child:rootAccount.GetChildren()) {
    //     std::cout << EncodeDestination(child) << std::endl;
    //     CManagedAccountData childAccount;
    //     db.GetAccountByAddress(child, childAccount);
    //     vertex_t childNode = boost::add_vertex(Vertex{ValueFromRoles(childAccount.GetRoles()).get_str()}, g);
    //     boost::add_edge(rootNode, childNode, g);
    // }
    if (rootAccount.GetChildren().size() > 0){
        LoadGraphChildren(rootAccount.GetChildren(), rootNode);
    }

    return true;
}

void CAccountDataVisualization::LoadGraphChildren(std::vector <CTxDestination> accountChildren, Vertex parentNode){

    for(const auto& child:accountChildren) {
        // std::cout << EncodeDestination(child) << std::endl;
        CManagedAccountData childAccount;
        db.GetAccountByAddress(child, childAccount);
        Vertex childNode = boost::add_vertex(VertexProperties{EncodeDestination(child),ValueFromRoles(childAccount.GetRoles()).get_str()}, g);
        boost::add_edge(parentNode, childNode, g);

        if (childAccount.GetChildren().size() > 0){
            LoadGraphChildren(childAccount.GetChildren(), childNode);
        }
        
    }

}

std::string CAccountDataVisualization::VisualizeGraph(){
    std::stringstream stringStream;

    boost::write_graphviz(stringStream, g, [&] (std::ostream& out, Vertex v) {
       out << "[address=\"" << g[v].address << "\"] [label=\"" << g[v].roles << "\"]";
    });

//    std::cout << stringStream.str() << std::endl << std::flush;
    return stringStream.str();
}

// CAccountDataVisualization::Graph CAccountDataVisualization::getGraph(){
//     return Graph;
// }


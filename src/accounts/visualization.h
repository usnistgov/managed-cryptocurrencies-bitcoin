// Copyright (c) 2018-2019 National Institute of Standards and Technology
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_ACCOUNT_GRAPH_H
#define BITCOIN_ACCOUNT_GRAPH_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <iostream>
#include <accounts/data.h>
#include <accounts/db.h>

using namespace boost;

class CAccountDataVisualization {
public:
    CAccountDataVisualization() {}

    CAccountDataVisualization(CManagedAccountDB accountDB) {
        db = accountDB;
        // TODO check that the db is set.
        std::cout << "There are " << db.size() << " nodes to graph" << std::endl;
        if (db.size() > 0){
            LoadGraph();
        }
    }

    std::string VisualizeGraph();
    // CAccountDataVisualization::Graph getGraph();

private:
    CManagedAccountDB db;
    
    struct VertexProperties { 
        std::string address;
        std::string roles; 
        };
    struct EdgeProperties { std::string name; };

    using Graph  = adjacency_list<listS, vecS, directedS, VertexProperties, EdgeProperties >;
    using Vertex = graph_traits<Graph>::vertex_descriptor;
    using Edge   = graph_traits<Graph>::edge_descriptor;

    //Instantiate a graph
    Graph g;

    bool LoadGraph();
    void LoadGraphChildren(std::vector <CTxDestination> accountChildren, Vertex parentNode);
};

#endif // BITCOIN_ACCOUNT_GRAPH_H
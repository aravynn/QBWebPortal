#include "QBXMLSync.h"

void QBXMLSync::updateInventoryMinMax(std::string ListID, int min)
{
    // --------------------------------------------------------------------------------------------------------------------!!
}

QBXMLSync::QBXMLSync(std::string QBID, std::string appName, std::string password) : m_req{ QBRequest(QBID, appName) }, m_sql{ SQLControl(password) }
{
    // check if a connection was indeed created, and manage it. This will change completely in the UI phase.
    if (!m_sql.connected) {
        // generate a connection file, if one does not exist. 
        std::cout << "Connection Generator\n" <<
            "--------------------\n" <<
            "SQL Username: ";

        std::string user, pass, ip, db, xmlPass;
        std::cin >> user;
        std::cin.clear();
        std::cin.ignore(INT_MAX, '\n');

        std::cout << "\nSQL Password: ";
        std::cin >> pass;
        std::cin.clear();
        std::cin.ignore(INT_MAX, '\n');

        std::cout << "\nSQL IP Location: ";
        std::cin >> ip;
        std::cin.clear();
        std::cin.ignore(INT_MAX, '\n');

        std::cout << "\nSQL Database: ";
        std::cin >> db;
        std::cin.clear();
        std::cin.ignore(INT_MAX, '\n');

        std::cout << "\nApp Password: ";
        std::cin >> xmlPass;
        std::cin.clear();
        std::cin.ignore(INT_MAX, '\n');

        std::cout << "\n\n\n";

        m_sql.generateConnectionFile(user, pass, ip, db, xmlPass); // ("QBXML", "cVyio%90pxE4", "tcp://127.0.0.1:3306", "qbxmlstorage", "TestPassword");
    }
}


bool QBXMLSync::getIteratorData(XMLNode& nodeRet, std::list<int> path, std::string& iteratorID, int& statusCode, int& remainingCount) {
    XMLNode BaseNode;
    
    if (nodeRet.getNodeFromPath(path, BaseNode)) {

        std::string name{ "iteratorID" };
        XMLNode attr = BaseNode.getAttribute(name);
        iteratorID.clear();
        iteratorID = attr.getStringData();

        name = "statusCode";
        attr = BaseNode.getAttribute(name);
        statusCode = attr.getIntData();

        name = "iteratorRemainingCount";
        attr = BaseNode.getAttribute(name);
        remainingCount = attr.getIntData();

        return true;
    }
    
    return false; //(failed to get data.)

}

// FOR ALL ITERATIVE FUNCTIONS. either create or run an iteration on the DB. returns number of row remaining or -1 for bad request.

//bool iterate(std::string& returnData, std::string table, int maximum = 100, bool useOwner = false, std::string iteratorID = "INIT");
int QBXMLSync::iterate(std::string& returnData, std::string& iteratorID, std::string table, int maximum, std::string date, bool useOwner, bool modDateFilter) {
    
    // steps: 
    // start the iterator with the initial call.
    // create the XML
    XMLNode node(table);

    if (iteratorID.size() > 0 && iteratorID.at(0) == '{') {
        node.addAttribute("iterator", "Continue"); // Start or Continue.
        node.addAttribute("iteratorID", iteratorID);
    }
    else {
        node.addAttribute("iterator", "Start"); // Start or Continue.
    }

    node.addNode("MaxReturned", std::to_string(maximum), std::list<int>{});
    
    if (date != "NA") {
        // non default should be legitimate value
        if (modDateFilter) {
            std::list<int> li;

            li.push_back(node.addNode(std::string("ModifiedDateRangeFilter"), li));

            node.addNode("FromModifiedDate", date, li);
        }
        else {
            node.addNode("FromModifiedDate", date, std::list<int>{});
        }
    }

    if (useOwner) {
        node.addNode("OwnerID", "0", std::list<int>{});
    }

 //   std::cout << XMLParser::createXMLRequest(node) << '\n';

    
    std::string xml = XMLParser::createXMLRequest(node);

    // send to QB, and get return value.
    m_req.processRequest(xml);

    returnData.clear();
    returnData = m_req.getResponse();

    XMLParser p(returnData);

    XMLNode nodeRet = p.getNode();

//    nodeRet.write();

    if (p.errorStatus()) {
        std::cout << "\n" << returnData << "\n\n";
        nodeRet.write();
    //   std::cout << '\n';
    }

    std::list<int> path = { 0, 0, 0 };

    XMLNode BaseNode;
    int remainingCount{ -1 };       // remaining count, used repeptitively.
    int statusCode{ 0 };            // status, used frequently.

    if (!getIteratorData(nodeRet, path, iteratorID, statusCode, remainingCount)) {
        std::cout << "Failed at get node \n";
        return -10; //failed to get data.
    }

//    std::cout << remainingCount << ' ' << statusCode << '\n';

    if (statusCode != 0) {
        nodeRet.getNodeFromPath(path, BaseNode);
        // output the error here. 
        std::string name{ "statusMessage" };
        XMLNode attr = BaseNode.getAttribute(name);

        std::cerr << "QBXML Error Code: " << statusCode << ": " << attr.getStringData() << '\n';
        return -100000 + statusCode;
    }
  
    return remainingCount;

}

// Add an address as provided by any other function, return the ID of the entered line. 
// ------------------------------------------------------------------------------------

bool QBXMLSync::queryAll(std::string& returnData, std::string table)
{
    // steps: 
    // start the iterator with the initial call.
    // create the XML
    XMLNode node(table);

//    std::cout << XMLParser::createXMLRequest(node) << '\n';

    std::string xml = XMLParser::createXMLRequest(node);

    // send to QB, and get return value.
    m_req.processRequest(xml);

    returnData.clear();
    returnData = m_req.getResponse();

    XMLParser p(returnData);

    XMLNode nodeRet = p.getNode();

    if (p.errorStatus()) {
        std::cout << "\n" << returnData << "\n\n";
        nodeRet.write();
        //   std::cout << '\n';
    }

    std::list<int> path = { 0, 0, 0 };

    XMLNode BaseNode;
    
    nodeRet.getNodeFromPath(path, BaseNode);

    std::string name = "statusCode";
    XMLNode attr = BaseNode.getAttribute(name);
    int statusCode = attr.getIntData();

    if (statusCode != 0) {
        // output the error here. 
        std::string name{ "statusMessage" };
        XMLNode attr = BaseNode.getAttribute(name);

        std::cerr << "QBXML Error Code: " << statusCode << ": " << attr.getStringData() << '\n';
        return false;
    }
    return true;
}

int QBXMLSync::addAddress(XMLNode& addressNode, std::string customerListID)
{
    std::string table = "address";

    std::vector<std::string> columns = { "ID" }; // just getting the ID.

    FilterPass inserts;
    inserts.resize(10);
    inserts.at(0) = { "CustomerListID", customerListID };
    inserts.at(1) = { "Address1", addressNode.getChildValue("Addr1", "") };
    inserts.at(2) = { "Address2", addressNode.getChildValue("Addr2", "") };
    inserts.at(3) = { "Address3", addressNode.getChildValue("Addr3", "") };
    inserts.at(4) = { "Address4", addressNode.getChildValue("Addr4", "") };
    inserts.at(5) = { "Address5", addressNode.getChildValue("Addr5", "") };
    inserts.at(6) = { "City", addressNode.getChildValue("City", "") };
    inserts.at(7) = { "State", addressNode.getChildValue("State", "") };
    inserts.at(8) = { "Country", addressNode.getChildValue("Country", "") };
    inserts.at(9) = { "PostalCode", addressNode.getChildValue("PostalCode", "") };

    // first, check if this ALREADY exists, and return that.
    if (m_sql.SQLSelect(columns, table, inserts, 1)) {
        m_sql.nextRow();
        return m_sql.getInt("ID");
    }

    // if it does not, then add and return THAT value.
    if (m_sql.SQLInsert(table, inserts)) {
        // inserted, so return the line ID.

        if (m_sql.SQLSelect(columns, table, inserts, 1)) {
            m_sql.nextRow();
            return m_sql.getInt("ID");
        } 
    }

    return -1; // failed for some reason.
}

// Pull inventory data, and parse it for insert into DB. 
// -----------------------------------------------------

bool QBXMLSync::getInventory()
{
    std::string data;
    std::string iterator;

    int loopLimit = 0;

    std::string time = m_sql.getConfigTime("inventory");

    std::list<int> path = { 0, 0, 0 }; // path direct to top node of data.

    while (iterate(data, iterator, "ItemQueryRq", 100, time, true) > 0) {
        std::cout << "No: " << ++loopLimit << " Datasize: " << data.size() << '\r';

        // format data for insert into database.
        XMLParser p(data);
        XMLNode node = p.getNode();
        XMLNode nodeChildren;
        node.getNodeFromPath(path, nodeChildren);

        bool findErr = false;

        int nodeCount = nodeChildren.getChildCount();
        for (int i{ 0 }; i < nodeCount; ++i) {
            // for each child, convert and output. 
            XMLNode child = nodeChildren.child(i);
            //std::cout << child.getName() << ' ' << child.getChildValue("Name", "") << '\n';

            std::string table{"inventory"};
            FilterPass inserts;
            FilterPass filter;
            inserts.resize(19); // all the data we'll need to pass.
            inserts.at(0) = { "ListID", child.getChildValue("ListID", "") };
            filter.push_back({ "ListID", child.getChildValue("ListID", "") });
            inserts.at(1) = { "TimeCreated", child.getChildValue("TimeCreated", "") };
            inserts.at(2) = { "TimeModified", child.getChildValue("TimeModified", "") };
            inserts.at(3) = { "Name", child.getChildValue("Name", "") };
            inserts.at(4) = { "SKU", child.getChildValue("FullName", "") };
            inserts.at(5) = { "ProductType", child.getName() };
            
            inserts.at(11) = { "Details", child.getChildValue("SalesDesc", "") };
            inserts.at(12) = { "PurchaseOrderDetails", child.getChildValue("PurchaseDesc", "") };

            std::string AsstName = "UnitOfMeasureSetRef";
            XMLNode Asst = child.findChild(AsstName);
            if (Asst.getStatus()) {
                inserts.at(13) = { "UOfM", Asst.getChildValue("FullName", "") };
            }
            else {
                inserts.at(13) = { "UOfM", std::string("") };
            }
            
            inserts.at(6) = { "InStock", child.getDoubleOrInt("QuantityOnHand")};

            inserts.at(7) = { "OnOrder", child.getDoubleOrInt("QuantityOnOrder") };
            inserts.at(8) = { "OnSalesOrder", child.getDoubleOrInt("QuantityOnSalesOrder") };
            inserts.at(9) = { "Price", child.getDoubleOrInt("SalesPrice") };
            inserts.at(10) = { "Cost", child.getDoubleOrInt("AverageCost") };
            inserts.at(15) = { "ReorderPoint", (long long)child.getIntOrDouble("ReorderPoint") };
            inserts.at(16) = { "MaxInv", (long long)child.getIntOrDouble("Max") };

            inserts.at(14) = { "IsActive", (long long)(child.getChildValue("IsActive", "") == "true" ? 1 : 0)};
            inserts.at(17) = { "RunningTally", (long long)0 };
            inserts.at(18) = { "EditSequence", child.getChildValue("EditSequence", "") };
            
            if (!m_sql.SQLUpdateOrInsert(table, inserts, filter)) {
                std::cout << "Insert/Update failed for " << inserts.at(3).second.str << '\n';
                continue; // skip the remainder of this line, as to not pointlessly fill the dataabase.
            }

            // if this is an assembly line, we'll need to also upload the lineitems.
            
            if (child.getName() == "ItemInventoryAssemblyRet") {
                // for each element, we'll go through and find the data required, and enter the new lines. 

                table = "assembly_lineitems";
                FilterPass childInserts;
                childInserts.resize(3);

                int childNodeCount = child.getChildCount();
                for (int i{ 0 }; i < childNodeCount; ++i) {
                    Asst = child.child(i);
                    if (Asst.getName() == "ItemInventoryAssemblyLine") {
                        // this line can be added to the assemblies.
                        FilterPass childFilter;

                        std::string list = "ItemInventoryRef";
                        XMLNode c = Asst.findChild(list);
                        childInserts.at(0) = { "ListID", c.getChildValue("ListID", "") };
                        childFilter.push_back({ "ListID", c.getChildValue("ListID", "") });
                        childInserts.at(1) = { "ReferenceListID", inserts.at(0).second.str };
                        childFilter.push_back({ "ReferenceListID", inserts.at(0).second.str });
                        childInserts.at(2) = { "Quantity", Asst.getDoubleOrInt("Quantity") };

                        if (!m_sql.SQLUpdateOrInsert(table, childInserts, childFilter)) {
                            std::cout << "Insert/Update AssemblyLine failed for " << inserts.at(3).second.str << '\n';
                        }
                    }
                }
            }  
        }
        //break;
        if (findErr) {
            std::cout << data << "\n\n\n";
            break;
        }
    }

    return m_sql.updateConfigTime("inventory");
}

bool QBXMLSync::getSalesOrders() 
{

    std::string data;
    std::string iterator;

    int loopLimit = 0;

    std::string time = m_sql.getConfigTime("salesorders");

    std::list<int> path = { 0, 0, 0 }; // path direct to top node of data.

    while (iterate(data, iterator, "SalesOrderQueryRq", 100, time, false, true) > 0) {
        std::cout << "No: " << ++loopLimit << " Datasize: " << data.size() << '\r';
        
        // format data for insert into database.
        XMLParser p(data);
        XMLNode node = p.getNode();
        XMLNode nodeChildren;
        node.getNodeFromPath(path, nodeChildren);

        bool findErr = false;

        // run though, set by set.
        int nodeCount = nodeChildren.getChildCount();
        for (int i{ 0 }; i < nodeCount; ++i) {
            // for each child, convert and output. 
            XMLNode child = nodeChildren.child(i);

            std::string table{ "orders" }; // GENERIC table used for all orders.
            FilterPass inserts;
            FilterPass filter;
            inserts.resize(26); // all the data we'll need to pass.
            inserts.at(0) = { "TxnID", child.getChildValue("TxnID", "") };
            filter.push_back({ "TxnID", child.getChildValue("TxnID", "") });
            inserts.at(1) = { "OrderType", std::string("SO") };
            inserts.at(2) = { "TimeCreated", child.getChildValue("TimeCreated", "") };
            inserts.at(3) = { "TimeModified", child.getChildValue("TimeModified", "") };
            inserts.at(4) = { "TxnDate", child.getChildValue("TxnDate", "1970-01-01") };
            inserts.at(5) = { "DueDate", child.getChildValue("DueDate", "1970-01-01") };
            inserts.at(6) = { "ShipDate", child.getChildValue("ShipDate", "1970-01-01") };

            XMLNode Asset = child.findChild("TermsRef");
            inserts.at(7) = { "TermsID", Asset.getChildValue("ListID", "") };
            Asset = child.findChild("CustomerRef");
            inserts.at(8) = { "CustomerListID", Asset.getChildValue("ListID", "") };
            inserts.at(16) = { "CustomerFullName", Asset.getChildValue("FullName", "") };
            Asset = child.findChild("ShipAddress");
            inserts.at(9) = { "ShipAddressID", (long long)addAddress(Asset, inserts.at(8).second.str) };
            Asset = child.findChild("BillAddress");
            inserts.at(10) = { "BillAddressID", (long long)addAddress(Asset, inserts.at(8).second.str) };
            Asset = child.findChild("ShipMethodRef");
            inserts.at(11) = { "ShipTypeID", Asset.getChildValue("ListID", "") };
            Asset = child.findChild("ItemSalesTaxRef");
            inserts.at(12) = { "TaxCodeID", Asset.getChildValue("ListID", "") };
            Asset = child.findChild("SalesRepRef");
            inserts.at(13) = { "SalesRepID", Asset.getChildValue("ListID", "") };

            inserts.at(14) = { "RefNumber", child.getChildValue("RefNumber", "") };
            inserts.at(15) = { "PONumber", child.getChildValue("PONumber", "") };
            inserts.at(17) = { "FOB", child.getChildValue("FOB", "") };
            inserts.at(18) = { "Subtotal", child.getDoubleOrInt("Subtotal") };
            inserts.at(19) = { "SalesTaxTotal", child.getDoubleOrInt("SalesTaxTotal") };
            inserts.at(20) = { "SaleTotal", child.getDoubleOrInt("TotalAmount") };
            inserts.at(21) = { "Balance", 0.0 };
            inserts.at(22) = { "IsPending", (long long)1 };
            inserts.at(23) = { "Status", (long long)(child.getChildValue("IsManuallyClosed", "") == "true" ? 0 : 1)};
            inserts.at(24) = { "IsInvoiced", (long long)(child.getChildValue("IsFullyInvoiced", "") == "true" ? 1 : 0)};
            inserts.at(25) = { "Notes", child.getChildValue("Memo", "") };

            if (!m_sql.SQLUpdateOrInsert(table, inserts, filter)) {
                std::cout << "Insert/Update failed for " << inserts.at(14).second.str << '\n';
            }

            // order lines and linked transaction inserting.
            int childNodeCount = child.getChildCount();
            for (int i{ 0 }; i < childNodeCount; ++i) {
                Asset = child.child(i);
                // Order Lines 
                if (Asset.getName() == "SalesOrderLineRet") {
                    
                    table = "lineitem";
                    FilterPass childInserts;
                    FilterPass childFilter;
                    childInserts.resize(10);
                    childInserts.at(0) = { "LineID", Asset.getChildValue("TxnLineID", "") };
                    childFilter.push_back({ "LineID", Asset.getChildValue("TxnLineID", "") });
                    childInserts.at(1) = { "TxnID", inserts.at(0).second.str };

                    XMLNode c = Asset.findChild("ItemRef");
                    childInserts.at(2) = { "ItemListID", c.getChildValue("ListID", "") };
                    childInserts.at(3) = { "Description", Asset.getChildValue("Desc", "") };
                    childInserts.at(4) = { "Quantity", Asset.getDoubleOrInt("Quantity") };
                    childInserts.at(5) = { "Rate", Asset.getDoubleOrInt("Rate") };
                    childInserts.at(6) = { "UnitOfMeasure", Asset.getChildValue("UnitOfMeasure", "") };

                    c = Asset.findChild("SalesTaxCodeRef");
                    childInserts.at(7) = { "TaxCode", c.getChildValue("ListID", "") };
                    childInserts.at(8) = { "UniqueIdentifier", (Asset.getChildValue("SerialNumber", "") != "" ? Asset.getChildValue("SerialNumber", "") : Asset.getChildValue("LotNumber", "") )};
                    childInserts.at(9) = { "Invoiced", (long long)Asset.getIntOrDouble("Invoiced") };
                    
                    if (!m_sql.SQLUpdateOrInsert(table, childInserts, childFilter)) {
                        std::cout << "Insert/Update Lineitem failed for " << inserts.at(3).second.str << '\n';
                    }
                }
                // Linked Transactions 
                if (Asset.getName() == "LinkedTxn") {

                    table = "orderlinks";
                    FilterPass childInserts;
                    childInserts.resize(3);
                    childInserts.at(0) = { "BaseID", inserts.at(0).second.str };
                    childInserts.at(1) = { "LinkID", Asset.getChildValue("TxnID", "") };
                    childInserts.at(2) = { "Type", Asset.getChildValue("TxnType", "") };

                    if (!m_sql.SQLUpdateOrInsert(table, childInserts, childInserts)) {
                        std::cout << "Insert/Update Linked Transaction failed for " << inserts.at(3).second.str << '\n';
                    }

                }

            }

        }
    }
    return m_sql.updateConfigTime("salesorders");
}

bool QBXMLSync::getEstimates() 
{

    std::string data;
    std::string iterator;

    int loopLimit = 0;

    std::string time = m_sql.getConfigTime("estimates");

    std::list<int> path = { 0, 0, 0 }; // path direct to top node of data.

    while (iterate(data, iterator, "EstimateQueryRq", 100, time, false, true) > 0) {
        std::cout << "No: " << ++loopLimit << " Datasize: " << data.size() << '\r';

        // format data for insert into database.
        XMLParser p(data);
        XMLNode node = p.getNode();
        XMLNode nodeChildren;
        node.getNodeFromPath(path, nodeChildren);

        bool findErr = false;

        // run though, set by set.
        int nodeCount = nodeChildren.getChildCount();
        for (int i{ 0 }; i < nodeCount; ++i) {
            // for each child, convert and output. 
            XMLNode child = nodeChildren.child(i);

            std::string table{ "orders" }; // GENERIC table used for all orders.
            FilterPass inserts;
            FilterPass filter;
            inserts.resize(26); // all the data we'll need to pass.
            inserts.at(0) = { "TxnID", child.getChildValue("TxnID", "") };
            filter.push_back({ "TxnID", child.getChildValue("TxnID", "") });
            inserts.at(1) = { "OrderType", std::string("ES") };
            inserts.at(2) = { "TimeCreated", child.getChildValue("TimeCreated", "") };
            inserts.at(3) = { "TimeModified", child.getChildValue("TimeModified", "") };
            inserts.at(4) = { "TxnDate", child.getChildValue("TxnDate", "1970-01-01") };
            inserts.at(5) = { "DueDate", child.getChildValue("DueDate", "1970-01-01") };
            inserts.at(6) = { "ShipDate", std::string("1970-01-01") };

            XMLNode Asset = child.findChild("TermsRef");
            inserts.at(7) = { "TermsID", Asset.getChildValue("ListID", "") };
            Asset = child.findChild("CustomerRef");
            inserts.at(8) = { "CustomerListID", Asset.getChildValue("ListID", "") };
            inserts.at(16) = { "CustomerFullName", Asset.getChildValue("FullName", "") };
            Asset = child.findChild("ShipAddress");
            inserts.at(9) = { "ShipAddressID", (long long)addAddress(Asset, inserts.at(8).second.str) };
            Asset = child.findChild("BillAddress");
            inserts.at(10) = { "BillAddressID", (long long)addAddress(Asset, inserts.at(8).second.str) };
            inserts.at(11) = { "ShipTypeID", std::string("")};
            Asset = child.findChild("ItemSalesTaxRef");
            inserts.at(12) = { "TaxCodeID", Asset.getChildValue("ListID", "") };
            Asset = child.findChild("SalesRepRef");
            inserts.at(13) = { "SalesRepID", Asset.getChildValue("ListID", "") };

            inserts.at(14) = { "RefNumber", child.getChildValue("RefNumber", "") };
            inserts.at(15) = { "PONumber", child.getChildValue("PONumber", "") };
            inserts.at(17) = { "FOB", child.getChildValue("FOB", "") };
            inserts.at(18) = { "Subtotal", child.getDoubleOrInt("Subtotal") };
            inserts.at(19) = { "SalesTaxTotal", child.getDoubleOrInt("SalesTaxTotal") };
            inserts.at(20) = { "SaleTotal", child.getDoubleOrInt("TotalAmount") };
            inserts.at(21) = { "Balance", 0.0 };
            inserts.at(22) = { "IsPending", (long long)1 };
            inserts.at(23) = { "Status", (long long)(child.getChildValue("IsActive", "") == "true" ? 0 : 1) };
            inserts.at(24) = { "IsInvoiced", (long long)1 };
            inserts.at(25) = { "Notes", child.getChildValue("Memo", "") };

            if (!m_sql.SQLUpdateOrInsert(table, inserts, filter)) {
                std::cout << "Insert/Update failed for " << inserts.at(14).second.str << '\n';
            }

            // order lines and linked transaction inserting.
            int childNodeCount = child.getChildCount();
            for (int i{ 0 }; i < childNodeCount; ++i) {
                Asset = child.child(i);
                // Order Lines 
                if (Asset.getName() == "EstimateLineRet") {

                    table = "lineitem";
                    FilterPass childInserts;
                    FilterPass childFilter;
                    childInserts.resize(10);
                    childInserts.at(0) = { "LineID", Asset.getChildValue("TxnLineID", "") };
                    childFilter.push_back({ "LineID", Asset.getChildValue("TxnLineID", "") });
                    childInserts.at(1) = { "TxnID", inserts.at(0).second.str };

                    XMLNode c = Asset.findChild("ItemRef");
                    childInserts.at(2) = { "ItemListID", c.getChildValue("ListID", "") };
                    childInserts.at(3) = { "Description", Asset.getChildValue("Desc", "") };
                    childInserts.at(4) = { "Quantity", Asset.getDoubleOrInt("Quantity") };
                    childInserts.at(5) = { "Rate", Asset.getDoubleOrInt("Rate") };
                    childInserts.at(6) = { "UnitOfMeasure", Asset.getChildValue("UnitOfMeasure", "") };

                    c = Asset.findChild("SalesTaxCodeRef");
                    childInserts.at(7) = { "TaxCode", c.getChildValue("ListID", "") };
                    childInserts.at(8) = { "UniqueIdentifier", std::string("")};
                    childInserts.at(9) = { "Invoiced", (long long)0 };

                    if (!m_sql.SQLUpdateOrInsert(table, childInserts, childFilter)) {
                        std::cout << "Insert/Update Lineitem failed for " << inserts.at(3).second.str << '\n';
                    }
                }
                // Linked Transactions 
                if (Asset.getName() == "LinkedTxn") {

                    table = "orderlinks";
                    FilterPass childInserts;
                    childInserts.resize(3);
                    childInserts.at(0) = { "BaseID", inserts.at(0).second.str };
                    childInserts.at(1) = { "LinkID", Asset.getChildValue("TxnID", "") };
                    childInserts.at(2) = { "Type", Asset.getChildValue("TxnType", "") };

                    if (!m_sql.SQLUpdateOrInsert(table, childInserts, childInserts)) {
                        std::cout << "Insert/Update Linked Transaction failed for " << inserts.at(3).second.str << '\n';
                    }

                }

            }

        }
    }
    return m_sql.updateConfigTime("estimates");
}

bool QBXMLSync::getInvoices() 
{

    std::string data;
    std::string iterator;

    int loopLimit = 0;

    std::string time = m_sql.getConfigTime("invoices");

    std::list<int> path = { 0, 0, 0 }; // path direct to top node of data.

    while (iterate(data, iterator, "InvoiceQueryRq", 100, time, false, true) > 0) {
        std::cout << "No: " << ++loopLimit << " Datasize: " << data.size() << '\r';

        // format data for insert into database.
        XMLParser p(data);
        XMLNode node = p.getNode();
        XMLNode nodeChildren;
        node.getNodeFromPath(path, nodeChildren);

        bool findErr = false;

        // run though, set by set.
        int nodeCount = nodeChildren.getChildCount();
        for (int i{ 0 }; i < nodeCount; ++i) {
            // for each child, convert and output. 
            XMLNode child = nodeChildren.child(i);

            std::string table{ "orders" }; // GENERIC table used for all orders.
            FilterPass inserts;
            FilterPass filter;
            inserts.resize(26); // all the data we'll need to pass.
            inserts.at(0) = { "TxnID", child.getChildValue("TxnID", "") };
            filter.push_back({ "TxnID", child.getChildValue("TxnID", "") });
            inserts.at(1) = { "OrderType", std::string("IN") };
            inserts.at(2) = { "TimeCreated", child.getChildValue("TimeCreated", "") };
            inserts.at(3) = { "TimeModified", child.getChildValue("TimeModified", "") };
            inserts.at(4) = { "TxnDate", child.getChildValue("TxnDate", "1970-01-01") };
            inserts.at(5) = { "DueDate", child.getChildValue("DueDate", "1970-01-01") };
            inserts.at(6) = { "ShipDate", child.getChildValue("ShipDate", "1970-01-01") };

            XMLNode Asset = child.findChild("TermsRef");
            inserts.at(7) = { "TermsID", Asset.getChildValue("ListID", "") };
            Asset = child.findChild("CustomerRef");
            inserts.at(8) = { "CustomerListID", Asset.getChildValue("ListID", "") };
            inserts.at(16) = { "CustomerFullName", Asset.getChildValue("FullName", "") };
            Asset = child.findChild("ShipAddress");
            inserts.at(9) = { "ShipAddressID", (long long)addAddress(Asset, inserts.at(8).second.str) };
            Asset = child.findChild("BillAddress");
            inserts.at(10) = { "BillAddressID", (long long)addAddress(Asset, inserts.at(8).second.str) };
            Asset = child.findChild("ShipMethodRef");
            inserts.at(11) = { "ShipTypeID", Asset.getChildValue("ListID", "") };
            Asset = child.findChild("ItemSalesTaxRef");
            inserts.at(12) = { "TaxCodeID", Asset.getChildValue("ListID", "") };
            Asset = child.findChild("SalesRepRef");
            inserts.at(13) = { "SalesRepID", Asset.getChildValue("ListID", "") };

            inserts.at(14) = { "RefNumber", child.getChildValue("RefNumber", "") };
            inserts.at(15) = { "PONumber", child.getChildValue("PONumber", "") };
            inserts.at(17) = { "FOB", child.getChildValue("FOB", "") };
            inserts.at(18) = { "Subtotal", child.getDoubleOrInt("Subtotal") };
            inserts.at(19) = { "SalesTaxTotal", child.getDoubleOrInt("SalesTaxTotal") };
            inserts.at(20) = { "SaleTotal", child.getDoubleOrInt("AppliedAmount") };
            inserts.at(21) = { "Balance", child.getDoubleOrInt("BalanceRemaining")  };     
            inserts.at(22) = { "IsPending", (long long)(child.getChildValue("IsPending", "") == "true" ? 0 : 1) };
            inserts.at(23) = { "Status", (long long)(child.getChildValue("IsPaid", "") == "true" ? 0 : 1) };
            inserts.at(24) = { "IsInvoiced", (long long)1 };
            inserts.at(25) = { "Notes", child.getChildValue("Memo", "") };

            if (!m_sql.SQLUpdateOrInsert(table, inserts, filter)) {
                std::cout << "Insert/Update failed for " << inserts.at(14).second.str << '\n';
            }

            // order lines and linked transaction inserting.
            int childNodeCount = child.getChildCount();
            for (int i{ 0 }; i < childNodeCount; ++i) {
                Asset = child.child(i);
                // Order Lines 
                if (Asset.getName() == "InvoiceLineRet") {

                    table = "lineitem";
                    FilterPass childInserts;
                    FilterPass childFilter;
                    childInserts.resize(10);
                    childInserts.at(0) = { "LineID", Asset.getChildValue("TxnLineID", "") };
                    childFilter.push_back({ "LineID", Asset.getChildValue("TxnLineID", "") });
                    childInserts.at(1) = { "TxnID", inserts.at(0).second.str };

                    XMLNode c = Asset.findChild("ItemRef");
                    childInserts.at(2) = { "ItemListID", c.getChildValue("ListID", "") };
                    childInserts.at(3) = { "Description", Asset.getChildValue("Desc", "") };
                    childInserts.at(4) = { "Quantity", Asset.getDoubleOrInt("Quantity") };
                    childInserts.at(5) = { "Rate", Asset.getDoubleOrInt("Rate") };
                    childInserts.at(6) = { "UnitOfMeasure", Asset.getChildValue("UnitOfMeasure", "") };

                    c = Asset.findChild("SalesTaxCodeRef");
                    childInserts.at(7) = { "TaxCode", c.getChildValue("ListID", "") };
                    childInserts.at(8) = { "UniqueIdentifier", (Asset.getChildValue("SerialNumber", "") != "" ? Asset.getChildValue("SerialNumber", "") : Asset.getChildValue("LotNumber", "")) };
                    childInserts.at(9) = { "Invoiced", (long long)0 };

                    if (!m_sql.SQLUpdateOrInsert(table, childInserts, childFilter)) {
                        std::cout << "Insert/Update Lineitem failed for " << inserts.at(3).second.str << '\n';
                    }
                }
                // Linked Transactions 
                if (Asset.getName() == "LinkedTxn") {

                    table = "orderlinks";
                    FilterPass childInserts;
                    childInserts.resize(3);
                    childInserts.at(0) = { "BaseID", inserts.at(0).second.str };
                    childInserts.at(1) = { "LinkID", Asset.getChildValue("TxnID", "") };
                    childInserts.at(2) = { "Type", Asset.getChildValue("TxnType", "") };

                    if (!m_sql.SQLUpdateOrInsert(table, childInserts, childInserts)) {
                        std::cout << "Insert/Update Linked Transaction failed for " << inserts.at(3).second.str << '\n';
                    }

                }

            }

        }
    }
    return m_sql.updateConfigTime("invoices");
}

bool QBXMLSync::getCustomers() 
{
    std::string data;
    std::string iterator;

    int loopLimit = 0;

    std::string time = m_sql.getConfigTime("customers");

    std::list<int> path = { 0, 0, 0 }; // path direct to top node of data.

    while (iterate(data, iterator, "CustomerQueryRq", 100, time) > 0) {
        std::cout << "No: " << ++loopLimit << " Datasize: " << data.size() << '\r';

        // format data for insert into database.
        XMLParser p(data);
        XMLNode node = p.getNode();
        XMLNode nodeChildren;
        node.getNodeFromPath(path, nodeChildren);

        bool findErr = false;

        // run though, set by set.
        int nodeCount = nodeChildren.getChildCount();
        for (int i{ 0 }; i < nodeCount; ++i) {
            // for each child, convert and output. 
            XMLNode child = nodeChildren.child(i);

            std::string table{ "customers" }; // GENERIC table used for all orders.
            FilterPass inserts;
            FilterPass filter;
            inserts.resize(18); // all the data we'll need to pass.
            inserts.at(0) = { "ListID", child.getChildValue("ListID", "") };
            filter.push_back({ "ListID", child.getChildValue("ListID", "") });
            inserts.at(1) = { "TimeCreated", child.getChildValue("TimeCreated", "") };
            inserts.at(2) = { "TimeModified", child.getChildValue("TimeModified", "") };

            XMLNode Asset = child.findChild("ShipAddress");
            inserts.at(3) = { "ShipAddressID", (long long)addAddress(Asset, inserts.at(0).second.str) };
            Asset = child.findChild("BillAddress");
            inserts.at(4) = { "BillAddressID", (long long)addAddress(Asset, inserts.at(0).second.str) };
            Asset = child.findChild("TermsRef");
            inserts.at(5) = { "TermsID", Asset.getChildValue("ListID", "") };
            Asset = child.findChild("ItemSalesTaxRef");
            inserts.at(6) = { "TaxCodeID", Asset.getChildValue("ListID", "") };
            Asset = child.findChild("SalesRepRef");
            inserts.at(7) = { "SalesRepID", Asset.getChildValue("ListID", "") };
            Asset = child.findChild("CustomerTypeRef");
            inserts.at(8) = { "PriceLevelID", Asset.getChildValue("ListID", "") };
            
            inserts.at(9) = { "isActive", (long long)(child.getChildValue("IsActive", "") == "true" ? 1 : 0) };
            inserts.at(10) = { "FullName", child.getChildValue("FullName", "") };
            inserts.at(11) = { "CompanyName", child.getChildValue("CompanyName", "") };
            inserts.at(12) = { "PhoneNumber", child.getChildValue("Phone", "") };
            inserts.at(13) = { "AltPhoneNumber", child.getChildValue("AltPhone", "") };
            inserts.at(14) = { "FaxNumber", child.getChildValue("Fax", "") };
            inserts.at(15) = { "EmailAddress", child.getChildValue("Email", "") };
            inserts.at(16) = { "Balance", child.getChildValue("Balance", 0.0) };
            inserts.at(17) = { "CreditLimit", child.getChildValue("CreditLimit", 0.0) };

            if (!m_sql.SQLUpdateOrInsert(table, inserts, filter)) {
                std::cout << "Insert/Update failed for " << inserts.at(14).second.str << '\n';
            }

            // insert all additional addresses 
            int childNodeCount = child.getChildCount();
            for (int i{ 0 }; i < childNodeCount; ++i) {
                Asset = child.child(i);
                if (Asset.getName() == "ShipToAddress") {
                    addAddress(Asset, inserts.at(10).second.str);
                }
            }
        }
    }

    return m_sql.updateConfigTime("customers");
}

bool QBXMLSync::getPriceLevels() 
{
    //------------------------------------------------------------------------------------------------------------------------!!
    std::string data;
    std::string iterator;

    int loopLimit = 0;

    std::list<int> path = { 0, 0, 0 }; // path direct to top node of data.

    if (queryAll(data, "CustomerTypeQueryRq")) {
        std::cout << "No: " << ++loopLimit << " Datasize: " << data.size() << '\r';

        // format data for insert into database.
        XMLParser p(data);
        XMLNode node = p.getNode();
        XMLNode nodeChildren;
        node.getNodeFromPath(path, nodeChildren);

        bool findErr = false;

        int nodeCount = nodeChildren.getChildCount();
        for (int i{ 0 }; i < nodeCount; ++i) {

            XMLNode child = nodeChildren.child(i);

            std::string table{ "price_level" }; // GENERIC table used for all orders.
            FilterPass inserts;
            FilterPass filter;
            inserts.resize(6); // all the data we'll need to pass.
            inserts.at(0) = { "ListID", child.getChildValue("ListID", "") };
            filter.push_back({ "ListID", child.getChildValue("ListID", "") });
            inserts.at(1) = { "TimeCreated", child.getChildValue("TimeCreated", "") };
            inserts.at(2) = { "TimeModified", child.getChildValue("TimeModified", "") };
            inserts.at(3) = { "isActive", (long long)(child.getChildValue("IsActive", "") == "true" ? 1 : 0) };
            inserts.at(4) = { "Name", child.getChildValue("Name", "") };
            inserts.at(5) = { "DiscountRate", 0.0 };

            if (!m_sql.SQLUpdateOrInsert(table, inserts, filter)) {
                std::cout << "Insert/Update failed for " << inserts.at(4).second.str << '\n';
            }

        }
    }
    else {
        return false;
    }
    return true;
}

bool QBXMLSync::getSalesTerms() 
{
    std::string data;
    std::string iterator;

    int loopLimit = 0;

    std::list<int> path = { 0, 0, 0 }; // path direct to top node of data.

    if(queryAll(data, "TermsQueryRq")) {
        std::cout << "No: " << ++loopLimit << " Datasize: " << data.size() << '\r';

        // format data for insert into database.
        XMLParser p(data);
        XMLNode node = p.getNode();
        XMLNode nodeChildren;
        node.getNodeFromPath(path, nodeChildren);

        bool findErr = false;

        int nodeCount = nodeChildren.getChildCount();
        for (int i{ 0 }; i < nodeCount; ++i) {

            XMLNode child = nodeChildren.child(i);

            std::string table{ "salesterms" }; // GENERIC table used for all orders.
            FilterPass inserts;
            FilterPass filter;
            inserts.resize(8); // all the data we'll need to pass.
            inserts.at(0) = { "ListID", child.getChildValue("ListID", "") };
            filter.push_back({ "ListID", child.getChildValue("ListID", "") });
            inserts.at(1) = { "TimeCreated", child.getChildValue("TimeCreated", "") };
            inserts.at(2) = { "TimeModified", child.getChildValue("TimeModified", "") };
            inserts.at(3) = { "isActive", (long long)(child.getChildValue("IsActive", "") == "true" ? 1 : 0) };
            inserts.at(4) = { "Name", child.getChildValue("Name", "") };
            inserts.at(5) = { "DaysDue", (long long)child.getChildValue("StdDueDays", 0) };
            inserts.at(6) = { "DiscountDays", (long long)child.getChildValue("StdDiscountDays", 0) };
            inserts.at(7) = { "DiscountRate", child.getChildValue("DiscountPct", 0.0) };

            if (!m_sql.SQLUpdateOrInsert(table, inserts, filter)) {
                std::cout << "Insert/Update failed for " << inserts.at(4).second.str << '\n';
            }
        }
    }
    else {
        return false;
    }
    return true; 
}

bool QBXMLSync::getTaxCodes() 
{
    std::string data;
    std::string iterator;

    int loopLimit = 0;

    std::list<int> path = { 0, 0, 0 }; // path direct to top node of data.

    if (queryAll(data, "SalesTaxCodeQueryRq")) {
        std::cout << "No: " << ++loopLimit << " Datasize: " << data.size() << '\r';

        // format data for insert into database.
        XMLParser p(data);
        XMLNode node = p.getNode();
        XMLNode nodeChildren;
        node.getNodeFromPath(path, nodeChildren);

        bool findErr = false;

        int nodeCount = nodeChildren.getChildCount();
        for (int i{ 0 }; i < nodeCount; ++i) {

            XMLNode child = nodeChildren.child(i);

            std::string table{ "taxcodes" }; // GENERIC table used for all orders.
            FilterPass inserts;
            FilterPass filter;
            inserts.resize(10); // all the data we'll need to pass.
            inserts.at(0) = { "ListID", child.getChildValue("ListID", "") };
            XMLNode Asset = child.findChild("ItemSalesTaxRef");
            filter.push_back({ "ListID", child.getChildValue("ListID", "") });
            inserts.at(1) = { "TaxID", Asset.getChildValue("ListID", "") };
            inserts.at(2) = { "Type", std::string("ST")};
            inserts.at(3) = { "TimeCreated", child.getChildValue("TimeCreated", "") };
            inserts.at(4) = { "TimeModified", child.getChildValue("TimeModified", "") };
            inserts.at(5) = { "isActive", (long long)(child.getChildValue("IsActive", "") == "true" ? 1 : 0) };
            inserts.at(6) = { "isTaxable", (long long)(child.getChildValue("IsTaxable", "") == "true" ? 1 : 0) };
            inserts.at(7) = { "Name", child.getChildValue("Name", "") };
            inserts.at(8) = { "Description", child.getChildValue("Desc", "")};
            inserts.at(9) = { "TaxRate", 0.0 };

            if (!m_sql.SQLUpdateOrInsert(table, inserts, filter)) {
                std::cout << "Insert/Update failed for " << inserts.at(7).second.str << '\n';
            }
        }
    }
    else {
        return false;
    }

    loopLimit = 0;

    if (queryAll(data, "ItemSalesTaxQueryRq")) {
        std::cout << "No: " << ++loopLimit << " Datasize: " << data.size() << '\r';

        // format data for insert into database.
        XMLParser p(data);
        XMLNode node = p.getNode();
        XMLNode nodeChildren;
        node.getNodeFromPath(path, nodeChildren);

        bool findErr = false;

        int nodeCount = nodeChildren.getChildCount();
        for (int i{ 0 }; i < nodeCount; ++i) {

            XMLNode child = nodeChildren.child(i);

            std::string table{ "taxcodes" }; // GENERIC table used for all orders.
            FilterPass inserts;
            FilterPass filter;
            inserts.resize(10); // all the data we'll need to pass.
            inserts.at(0) = { "ListID", child.getChildValue("ListID", "") };
            filter.push_back({ "ListID", child.getChildValue("ListID", "") });
            inserts.at(1) = { "TaxID", std::string("")};
            inserts.at(2) = { "Type", std::string("IT") };
            inserts.at(3) = { "TimeCreated", child.getChildValue("TimeCreated", "") };
            inserts.at(4) = { "TimeModified", child.getChildValue("TimeModified", "") };
            inserts.at(5) = { "isActive", (long long)(child.getChildValue("IsActive", "") == "true" ? 1 : 0) };
            inserts.at(6) = { "isTaxable", (long long)1 };
            inserts.at(7) = { "Name", child.getChildValue("Name", "") };
            inserts.at(8) = { "Description", child.getChildValue("ItemDesc", "")};
            inserts.at(9) = { "TaxRate", child.getDoubleOrInt("TaxRate") };

            if (!m_sql.SQLUpdateOrInsert(table, inserts, filter)) {
                std::cout << "Insert/Update failed for " << inserts.at(7).second.str << '\n';
            }
        }
    }
    else {
        return false;
    }
    return true;
}

bool QBXMLSync::getSalesReps() 
{
    std::string data;
    std::string iterator;

    int loopLimit = 0;

    std::list<int> path = { 0, 0, 0 }; // path direct to top node of data.

    if (queryAll(data, "SalesRepQueryRq")) {
        std::cout << "No: " << ++loopLimit << " Datasize: " << data.size() << '\r';

        // format data for insert into database.
        XMLParser p(data);
        XMLNode node = p.getNode();
        XMLNode nodeChildren;
        node.getNodeFromPath(path, nodeChildren);

        bool findErr = false;

        int nodeCount = nodeChildren.getChildCount();
        for (int i{ 0 }; i < nodeCount; ++i) {

            XMLNode child = nodeChildren.child(i);

            std::string table{ "salesreps" }; // GENERIC table used for all orders.
            FilterPass inserts;
            FilterPass filter;
            inserts.resize(6); // all the data we'll need to pass.
            inserts.at(0) = { "ListID", child.getChildValue("ListID", "") };
            filter.push_back({ "ListID", child.getChildValue("ListID", "") });
            inserts.at(1) = { "TimeCreated", child.getChildValue("TimeCreated", "") };
            inserts.at(2) = { "TimeModified", child.getChildValue("TimeModified", "") };
            inserts.at(3) = { "Initial", child.getChildValue("Initial", "") };
            inserts.at(4) = { "isActive", (long long)(child.getChildValue("IsActive", "") == "true" ? 1 : 0) };
            XMLNode Asset = child.findChild("SalesRepEntityRef");
            inserts.at(5) = { "Name", Asset.getChildValue("FullName", "") };

            if (!m_sql.SQLUpdateOrInsert(table, inserts, filter)) {
                std::cout << "Insert/Update failed for " << inserts.at(5).second.str << '\n';
            }
        }
    }
    else {
        return false;
    }
    return true;
}

void QBXMLSync::updateMinMax()
{
    std::cout << "\nMin/Max Update Not Set Up \n";

    // function - for each product, get all sales orders that the product was used on, as well as all assemblies that used this product. 
    std::vector<std::string> columns = { "ListID" };
    std::string table = "inventory";
    FilterPass filter;

    if (m_sql.SQLSelect(columns, table, filter)) {
        // gets all items in the DB, and returns them, one by one. 
        
        // first, get relevent stock data from  the DB, mainly the ListID for the related product. 
        std::string REQUEST = "";
        FilterPass inserts;
        if (m_sql.SQLComplex(REQUEST, inserts)) {

        }
    // next, get quantities from sales orders with the last year from NOW().
    // save the tally for this product, we have the information required for step 1. 
    
    }
    

    // part 2, new FOR loop, to go through all products again, except this time we'll be collecting and updating as we go. 
    // get all quantities and the count per assembly for all assemblies that use this product. 
    // calculate the annual usage, as a combination of all parts used plus all assemblies used. 
    // finally, update the running tally in the DB. 
    // check quickbooks for any issues with the current editsequence, confirm a correct passage of the existing variable. 
    // send an update to quickbooks, to update the list item in quickbooks. 
    // also, update the max to <max></max> allegedly this will solve the update issue.
}
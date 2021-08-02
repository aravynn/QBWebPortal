#include "SQLControl.h"

// Create all database tables in the case of failure no DB tables.
// ---------------------------------------------------------------
void SQLControl::initializeDatabase()
{
    // TO BE CREATED -----------------------------------------------------------------------------------------------------------!!
    std::cout << "Database not created. Creating... \n";

    // string for creating the DB.
   // Tables to add: 
   // Users
    std::string REQUEST{
        "CREATE TABLE users (\n"
        "ID int NOT NULL AUTO_INCREMENT,\n"
        "User char(200) NOT NULL DEFAULT '',\n"
        "Pass char(200) NOT NULL DEFAULT '',\n"
        "Salt char(200) NOT NULL DEFAULT '',\n"
        "CustomerID char(40) DEFAULT '',\n"
        "RepID int DEFAULT -1,\n"
        "PRIMARY KEY (ID)"
        ")"
    };

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable users \n"; // ensure no errors occur during request.
    }

    std::string table{ "users" };
    FilterPass inserts;
    inserts.resize(1);
    // forces a generic user. 
    inserts.at(0) = { std::string("User"), std::string("QBXML") };
    SQLInsert(table, inserts);

    // Inventory
    REQUEST =
        "CREATE TABLE inventory (\n"
        "ListID char(40) NOT NULL,\n"
        "TimeCreated datetime DEFAULT '1000-01-01 00:00:00',\n"
        "TimeModified datetime DEFAULT '1000-01-01 00:00:00',\n"
        "Name char(200) DEFAULT '',\n"
        "SKU char(200) DEFAULT '',\n"
        "ProductType char(60) DEFAULT '',\n"
        "InStock double DEFAULT 0.0,\n"
        "OnOrder double DEFAULT 0.0,\n"
        "OnSalesOrder double DEFAULT 0.0,\n"
        "Price double DEFAULT 0.0,\n"
        "Cost double DEFAULT 0.0, \n"
        "Details text,\n"
        "PurchaseOrderDetails text,\n"
        "UOfM char(100) DEFAULT 'Ea',\n"
        "IsActive tinyint DEFAULT 1,\n"
        "ReorderPoint int DEFAULT 0,\n"
        "MaxInv int DEFAULT 0,\n"
        "RunningTally double DEFAULT 0.0,\n"
        "EditSequence char(50) DEFAULT '',\n"
        "PRIMARY KEY (ListID)\n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable inventory \n"; // ensure no errors occur during request.
    }

    // Assembly Line Items
    REQUEST =
        "CREATE TABLE assembly_lineitems (\n"
        "ID int NOT NULL AUTO_INCREMENT,\n"
        "ListID char(40) NOT NULL,\n"
        "ReferenceListID char(40) NOT NULL,\n"
        "Quantity int DEFAULT 1, \n"
        "PRIMARY KEY (ID)\n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable assemblylineitems \n"; // ensure no errors occur during request.
    }

    // MinDivider & Content
    REQUEST =
        "CREATE TABLE minimumdivider (\n"
        "ID int NOT NULL AUTO_INCREMENT,\n"
        "Prefix char(2) NOT NULL DEFAULT '',\n"
        "Divider int NOT NULL DEFAULT 4, \n"
        "PRIMARY KEY (ID)\n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable minimumdivider \n"; // ensure no errors occur during request.
    }

    // add the dividers by default.
    std::vector<std::string> prefixes{ "02", "03", "04", "06", "09", "15", "23", "26", "31", "33", "35", "36", "38", "57" };
    inserts.resize(2);
    inserts.at(0) = { std::string("Prefix"), std::string("--") };
    inserts.at(1) = { std::string("Divider"), (long long)4 };

    table = "minimumdivider";

    for (std::string p : prefixes) {
        inserts.at(0).second.str = p;
        SQLInsert(table, inserts);
    }

    // Salesorders, estimates, invoices.
    REQUEST =
        "CREATE TABLE orders (\n"
        "TxnID char(40) NOT NULL,\n"
        "OrderType char(2) NOT NULL DEFAULT 'NA', \n" // SO, PO, or ES(timate)
        "TimeCreated datetime DEFAULT '1000-01-01 00:00:00', \n"
        "TimeModified datetime DEFAULT '1000-01-01 00:00:00', \n"
        "TxnDate date DEFAULT '1000-01-01', \n"
        "DueDate date DEFAULT '1000-01-01', \n"
        "ShipDate date DEFAULT '1000-01-01', \n"
        "TermsID char(40) NOT NULL DEFAULT '', \n"
        "CustomerListID char(40) NOT NULL, \n"
        "ShipAddressID int NOT NULL DEFAULT -1, \n"
        "BillAddressID int NOT NULL DEFAULT -1, \n"
        "ShipTypeID char(40) NOT NULL DEFAULT '', \n"
        "TaxCodeID char(40) NOT NULL DEFAULT '', \n"
        "SalesRepID char(40) NOT NULL DEFAULT '', \n"
        "RefNumber char(50) NOT NULL DEFAULT '', \n" // PO, SO, or invoice number
        "PONumber char(50) NOT NULL DEFAULT '', \n"
        "CustomerFullName char(255) DEFAULT '', \n"
        "FOB char(50) NOT NULL DEFAULT '', \n"
        "Subtotal double DEFAULT 0.0, \n"
        "SalesTaxTotal double DEFAULT 0.0, \n"
        "SaleTotal double DEFAULT 0.0, \n"
        "Balance double DEFAULT 0.0, \n"
        "IsPending int(1) NOT NULL DEFAULT 1, \n"
        "Status int(1) NOT NULL DEFAULT 1, \n" // isActive(1) or !isClosed(0),
        "IsInvoiced int (1) NOT NULL DEFAULT 0, \n"
        "Notes text, \n"
        "PRIMARY KEY (TxnID)\n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable orders \n"; // ensure no errors occur during request.
    }

    // SalesOrderLineitems
    REQUEST =
        "CREATE TABLE lineitem (\n"
        "LineID char(40) NOT NULL, \n"
        "TxnID char(40) NOT NULL, \n"
        "ItemListID char(40) NOT NULL, \n"
        "Description text, \n"
        "Quantity double DEFAULT 0, \n"
        "Rate double DEFAULT 0.0, \n"
        "UnitOfMeasure char(50) DEFAULT '', \n"
        "TaxCode char(40) NOT NULL DEFAULT '', \n"
        "UniqueIdentifier char(255) DEFAULT '', \n" // will be used for serial numbers, etc.
        "Invoiced int DEFAULT 0, \n" // only for sales orders. 
        "PRIMARY KEY (LineID)\n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable lineitem \n"; // ensure no errors occur during request.
    }

    // customers
    REQUEST =
        "CREATE TABLE customers (\n"
        "ListID char(40) NOT NULL, \n"
        "TimeCreated datetime DEFAULT '1000-01-01 00:00:00', \n"
        "TimeModified datetime DEFAULT '1000-01-01 00:00:00', \n"
        "ShipAddressID int NOT NULL DEFAULT -1, \n"
        "BillAddressID int NOT NULL DEFAULT -1, \n"
        "TermsID char(40) NOT NULL DEFAULT '', \n"
        "TaxCodeID char(40) NOT NULL DEFAULT '', \n"
        "SalesRepID char(40) NOT NULL DEFAULT '', \n"
        "PriceLevelID char(40) NOT NULL DEFAULT '', \n"
        "isActive int(1) NOT NULL DEFAULT 1, \n" // isActive    
        "FullName char(255) NOT NULL DEFAULT '', \n"
        "CompanyName char(255) NOT NULL DEFAULT '', \n"               
        "PhoneNumber char(30) DEFAULT '', \n"
        "AltPhoneNumber char(30) DEFAULT '', \n"
        "FaxNumber char(30) DEFAULT '', \n"
        "EmailAddress char(255) DEFAULT '', \n" // will be used to match when creating a new user.       
        "Balance double DEFAULT 0.0, \n"
        "CreditLimit double DEFAULT 0.0, \n"     
        "PRIMARY KEY (ListID) \n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable customers \n"; // ensure no errors occur during request.
    }


    // Price Level Rates
    REQUEST =
        "CREATE TABLE price_level (\n"
        "ListID char(40) NOT NULL, \n"
        "TimeCreated datetime DEFAULT '1000-01-01 00:00:00', \n"
        "TimeModified datetime DEFAULT '1000-01-01 00:00:00', \n"
        "isActive int(1) NOT NULL DEFAULT 1, \n" // isActive    
        "Name char(255) NOT NULL DEFAULT '', \n"
        "DiscountRate double DEFAULT 0.0, \n" //PriceLevelFixedPercentage
        "PRIMARY KEY (ListID) \n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable price_level \n"; // ensure no errors occur during request.
    }

    // Sales Terms
    REQUEST =
        "CREATE TABLE salesterms (\n"
        "ListID char(40) NOT NULL, \n"
        "TimeCreated datetime DEFAULT '1000-01-01 00:00:00', \n"
        "TimeModified datetime DEFAULT '1000-01-01 00:00:00', \n"
        "isActive int(1) NOT NULL DEFAULT 1, \n" // isActive    
        "Name char(255) NOT NULL DEFAULT '', \n"
        "DaysDue int DEFAULT 0, \n"
        "DiscountDays int DEFAULT 0, \n"
        "DiscountRate double DEFAULT 0.0, \n" //Discountpct
        "PRIMARY KEY (ListID) \n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable sales_terms \n"; // ensure no errors occur during request.
    }
    
    // Tax Codes
    REQUEST =
        "CREATE TABLE taxcodes (\n"
        "ListID char(40) NOT NULL, \n"
        "TaxID char(40) NOT NULL DEFAULT '', \n"
        "Type char(2) NOT NULL, \n" // ST for sales tax, IT for Item Tax
        "TimeCreated datetime DEFAULT '1000-01-01 00:00:00', \n"
        "TimeModified datetime DEFAULT '1000-01-01 00:00:00', \n"
        "isActive int(1) NOT NULL DEFAULT 1, \n" // isActive    
        "isTaxable int(1) NOT NULL DEFAULT 1, \n" // isActive    
        "Name char(255) NOT NULL DEFAULT '', \n"
        "Description text, \n"
        "TaxRate double NOT NULL DEFAULT 0.0, \n"
        "PRIMARY KEY (ListID) \n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable taxcodes \n"; // ensure no errors occur during request.
    }
    
    // Sales reps
    // Tax Codes
    REQUEST =
        "CREATE TABLE salesreps (\n"
        "ListID char(40) NOT NULL, \n"
        "TimeCreated datetime DEFAULT '1000-01-01 00:00:00', \n"
        "TimeModified datetime DEFAULT '1000-01-01 00:00:00', \n"
        "Initial char(20) NOT NULL DEFAULT '', \n"
        "isActive int(1) NOT NULL DEFAULT 1, \n" 
        "Name char(255) NOT NULL DEFAULT '', \n"
        "PRIMARY KEY (ListID) \n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable salesreps \n"; // ensure no errors occur during request.
    }

    // Shipping Type (+ tracking number)
    REQUEST =
        "CREATE TABLE shipmethod (\n"
        "ID int NOT NULL AUTO_INCREMENT,\n"
        "TxnID char(40) NOT NULL DEFAULT '', \n"
        "Name char(255) NOT NULL DEFAULT '', \n"
        "TrackingID char(255) DEFAULT '', \n"
        "PRIMARY KEY (ID) \n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable shipmethod \n"; // ensure no errors occur during request.
    }


    // Order Links (base to other order. this taxonomy allows for order relations on the far end.)
    REQUEST =
        "CREATE TABLE orderlinks (\n"
        "ID int NOT NULL AUTO_INCREMENT,\n"
        "BaseID char(40) NOT NULL DEFAULT '', \n"
        "LinkID char(40) NOT NULL DEFAULT '', \n"
        "Type char(255) NOT NULL DEFAULT '', \n"
        "PRIMARY KEY (ID) \n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable orderlinks \n"; // ensure no errors occur during request.
    }

    // Addresses
    REQUEST =
        "CREATE TABLE address (\n"
        "ID int NOT NULL AUTO_INCREMENT, \n"
        "CustomerListID char(40) NOT NULL, \n"
        "Address1 char(255) DEFAULT '', \n"
        "Address2 char(255) DEFAULT '', \n"
        "Address3 char(255) DEFAULT '', \n"
        "Address4 char(255) DEFAULT '', \n"
        "Address5 char(255) DEFAULT '', \n"
        "City char(100) DEFAULT '', \n"
        "State char(100) DEFAULT '', \n"
        "Country char(40) DEFAULT '', \n"
        "PostalCode char(20) DEFAULT '', \n"
        "PRIMARY KEY (ID)\n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable address \n"; // ensure no errors occur during request.
    }

    // SyncConfig
    REQUEST =
        "CREATE TABLE sync_config (\n"
        "ID int NOT NULL AUTO_INCREMENT, \n"
        "Time datetime DEFAULT '1970-01-01T12:00:00',\n"
        "ConfigKey char(200) NOT NULL, \n"
        "PRIMARY KEY (ID)\n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable sync_config \n"; // ensure no errors occur during request.
    }

    table = "sync_config";
    inserts.clear();
    inserts.resize(0);
    inserts.push_back({ "ConfigKey", std::string("inventory") });
    SQLInsert(table, inserts);
    inserts.at(0).second = std::string("customers");
    SQLInsert(table, inserts);
    inserts.at(0).second = std::string("purchaseorders");
    SQLInsert(table, inserts);
    inserts.at(0).second = std::string("salesorders");
    SQLInsert(table, inserts);
    inserts.at(0).second = std::string("invoices");
    SQLInsert(table, inserts);
    inserts.at(0).second = std::string("estimates");
    SQLInsert(table, inserts);


    // SyncQueue
    REQUEST =
        "CREATE TABLE sync_queue (\n"
        "ID int NOT NULL AUTO_INCREMENT, \n"
        "Action char(40) NOT NULL DEFAULT '', \n"
        "Ident char(40) NOT NULL DEFAULT '', \n"
        "Priority int DEFAULT 0, \n"
        "Status char(1) NOT NULL DEFAULT 'q', \n"
        "Message text, \n"
        "PRIMARY KEY (ID)\n"
        ")";

    m_stmt.reset(m_con->prepareStatement(REQUEST));
    if (m_stmt->execute()) {
        std::cerr << "ERROR @ CreateTable sync_queue \n"; // ensure no errors occur during request.
    }

    std::cout << "Database Creation Complete. \n";
}
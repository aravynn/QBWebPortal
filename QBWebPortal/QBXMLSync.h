#pragma once


/**
* 
* QBXMLSync controls the actual processing functions required for the transfer of data from Quickbooks to the database. 
* Additionally, this class will also manage any required inserts and updates to the file, as per the given data. 
* This class is unlikely to store a great deal of held information.
* 
*/

// std functions 
#include <vector>
#include <string>
#include <sstream> // for date generator.
#include <iomanip> // setw
#include <memory>  // weak, shared pointers.

// app classes. 
#include "SQLControl.h"
#include "XMLParser.h"
#include "XMLNode.h"
#include "QBRequest.h"

enum class SQLTable {
	inventory,
	customer,
	purchaseorder,
	salesorder,
	invoice,
	estimate
};

class QBXMLSync
{
private:
	SQLControl m_sql;	// connects to SQL and handles any collection and return. 
	QBRequest m_req;	// connects to QB and handles any collection and return. 
	std::shared_ptr<bool> m_isActive; // bool for determining the status of the run. will be defaulted to TRUE externally.

	void updateInventoryMinMax(std::string ListID, int min); // per line, update the min/max, to minimum, 0.

	bool getIteratorData(XMLNode& nodeRet, std::list<int> path, std::string& iteratorID, int& statusCode, int& remainingCount);
	int iterate(std::string& returnData, std::string& iteratorID, std::string table, int maximum = 100, std::string date = "NA", bool useOwner = false, bool modDateFilter = false, bool orderLinks = false);
	bool queryAll(std::string& returnData, std::string table);
	int addAddress(XMLNode& addressNode, std::string customerListID);

	bool isActive();
public:
	QBXMLSync(std::string QBID, std::string appName, std::string password, std::shared_ptr<bool> active = nullptr);	// generate the SQL and request items. 
	
	bool getInventory();	
	bool getInventory_old(); // Remove once testing confirmed. ---------------------------------------------------------------------------------------------!!

	bool getSalesOrders();
	bool getSalesOrders_old(); // Remove once testing confirmed. ---------------------------------------------------------------------------------------------!!

	bool getEstimates();
	bool getEstimates_old(); // Remove once testing confirmed. ---------------------------------------------------------------------------------------------!!

	bool getInvoices();
	bool getInvoices_old(); // Remove once testing confirmed. ---------------------------------------------------------------------------------------------!!

	bool getCustomers();
	bool getCustomers_old(); // for removal. . -----------------------!!

	bool getPriceLevels();
	bool getSalesTerms();
	bool getTaxCodes();
	bool getSalesReps();
//	bool getShippingTypes();	// not sure if required. 

	bool updateMinMax(); // update the minmax for all items. 
	bool updateMinMaxBatch(int batch = 100); // update the minmax for all items, in pre-grouped sets. This may not be at all valid. 
	bool updateMinMaxInventory(const std::string& listID, std::string& editSequence, int reorderPoint = -1, bool max = false);
	bool updateMinMaxInventory(const std::vector<std::string>& listID, std::vector<std::string>& editSequence, std::vector<int> newValue, bool max = false); // max is assumed for ALL items.

	bool timeReset(SQLTable table, int Y = 1970, int M = 1, int D = 1, int H = 12, int m = 0, int S = 0);
	bool timeReset(SQLTable table, std::string datetime = "1970-01-01 12:00:00");
};


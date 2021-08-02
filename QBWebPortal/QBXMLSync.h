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

// app classes. 
#include "SQLControl.h"
#include "XMLParser.h"
#include "XMLNode.h"
#include "QBRequest.h"

class QBXMLSync
{
private:
	SQLControl m_sql;	// connects to SQL and handles any collection and return. 
	QBRequest m_req;	// connects to QB and handles any collection and return. 

	void updateInventoryMinMax(std::string ListID, int min); // per line, update the min/max, to minimum, 0.

	bool getIteratorData(XMLNode& nodeRet, std::list<int> path, std::string& iteratorID, int& statusCode, int& remainingCount);
	int iterate(std::string& returnData, std::string& iteratorID, std::string table, int maximum = 100, std::string date = "NA", bool useOwner = false, bool modDateFilter = false);
	bool queryAll(std::string& returnData, std::string table);
	int addAddress(XMLNode& addressNode, std::string customerListID);
public:
	QBXMLSync(std::string QBID, std::string appName, std::string password);	// generate the SQL and request items. 
	
	bool getInventory();	
	bool getSalesOrders();
	bool getEstimates();
	bool getInvoices();
	bool getCustomers();
	bool getPriceLevels();
	bool getSalesTerms();
	bool getTaxCodes();
	bool getSalesReps();
//	bool getShippingTypes();	// not sure if required. 

	void updateMinMax(); // update the minmax for all items. 
};


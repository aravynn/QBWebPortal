// QBWebPortal.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>  // primarily for testing purposes, with XML sample.
#include <sstream>  // for testing purposes also.
#include <limits>
#include "XMLParser.h"
#include "XMLNode.h"
#include "Enums.h"
#include "ErrorReporting.h"

#include "QBRequest.h"

#include "SQLControl.h"

#include "QBXMLSync.h"

// Threading Stuff, a bit of a test...
//#include "qbsyncthreads.h"
//#include <thread>

int main()
{
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    std::cout << "Quickbooks Web Portal\n";
    std::cout << "---------------------\n";
    /*
    std::string tpass;
    std::cout << "\nPlease Enter Your Password: ";
    std::cin >> tpass;
    std::cin.clear();
    std::cin.ignore(INT_MAX, '\n');

    std::cout << "\n";
    */

    QBXMLSync qbs("QBX", "MEPBro Sampler", "TestPassword"); //tpass); // adding password coverage temporarily until we're ready for production values.

    std::cout << "Adding Inventory: \n"
                 "----------------- \n";
    qbs.getInventory();

    std::cout << "Adding Sales Prders: \n"
                 "------------------- \n";
    qbs.getSalesOrders();

    std::cout << "Adding Estimates: \n"
                 "----------------- \n";
    qbs.getEstimates();

    std::cout << "Adding Invoices: \n"
                 "---------------- \n";
    qbs.getInvoices();

    std::cout << "Adding Customers: \n"
                 "----------------- \n";
    qbs.getCustomers();

    std::cout << "Adding Price Levels: \n"
                 "-------------------- \n";
    qbs.getPriceLevels();

    std::cout << "Adding Sales Terms: \n"
                 "------------------- \n";
    qbs.getSalesTerms();

    std::cout << "Adding Tax Codes: \n"
                 "----------------- \n";
    qbs.getTaxCodes();

    std::cout << "Adding Sales Reps: \n"
                 "------------------ \n";
    qbs.getSalesReps();
    
    qbs.updateMinMax();
     //   sq.generateConnectionFile(user, pass, ip, db, xmlPass); // ("QBXML", "cVyio%90pxE4", "tcp://127.0.0.1:3306", "qbxmlstorage", "TestPassword");
   
    CoUninitialize();

    // Required Components: (Mark as completed)
    // ----------------------------------------
    // XML Converter (WORKING)
    // MySQL networked database.
    // SSL database support
    // Quickbooks SDK working (WORKING)
    // read/write data from quickbooks. (WORKING)
    // read/write to network DB
    // error log file for error reporting
    // local configuration file
    // PDF generation support
    // User interface, for large format developer. 
    // UI for PDF generator
    // UI for inventory reader. 
    // !!! --- Note: This can be build into 2 separate applications, if required. We don't need to have this as an "all in one" solution. --- !!!

    // Errors to Fix: 
    // --------------
    
}

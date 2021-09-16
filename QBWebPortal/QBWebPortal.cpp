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
    try {
        QBXMLSync qbs("QBX", "MEPBro Sampler", "TestPassword"); //tpass); // adding password coverage temporarily until we're ready for production values.

        qbs.timeReset(SQLTable::inventory, 2002, 03, 13, 19, 23, 45);

        std::cout << "Adding Inventory: \n"
            "----------------- \n";
        qbs.getInventory();

        std::cout << "Complete                                     \n";

        std::cout << "Adding Sales Orders: \n"
            "------------------- \n";
        qbs.getSalesOrders();

        std::cout << "Complete                                     \n";

        /*
        std::cout << "Adding Estimates: \n"
                     "----------------- \n";
        qbs.getEstimates();

        std::cout << "Complete                                     \n";

        std::cout << "Adding Invoices: \n"
                     "---------------- \n";
        qbs.getInvoices();

        std::cout << "Complete                                     \n";

        std::cout << "Adding Customers: \n"
                     "----------------- \n";
        qbs.getCustomers();

        std::cout << "Complete                                     \n";

        */

        std::cout << "Adding Price Levels: \n"
            "-------------------- \n";
        qbs.getPriceLevels();
        std::cout << "Complete                                     \n";

        std::cout << "Adding Sales Terms: \n"
            "------------------- \n";
        qbs.getSalesTerms();
        std::cout << "Complete                                     \n";

        std::cout << "Adding Tax Codes: \n"
            "----------------- \n";
        qbs.getTaxCodes();
        std::cout << "Complete                                     \n";

        std::cout << "Adding Sales Reps: \n"
            "------------------ \n";
        qbs.getSalesReps();
        std::cout << "Complete                                     \n";

        std::cout << "Update Min/Max: \n"
            "--------------- \n";

        // testing, do a BATCH update.
        qbs.updateMinMaxBatch(50);

        std::cout << "Complete                                     \n\n\n";
    }
    catch (ThrownError e) {
        std::cout << "Failed to load Quickbooks\n";
        SQLControl S("TestPassword");
        S.logError(e.error, e.data);
    }
        

     //   sq.generateConnectionFile(user, pass, ip, db, xmlPass); // ("QBXML", "cVyio%90pxE4", "tcp://127.0.0.1:3306", "qbxmlstorage", "TestPassword");
   
    CoUninitialize();

    return 0;
}

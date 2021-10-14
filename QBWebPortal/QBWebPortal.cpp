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

#include <stdlib.h> // sleep

#include <thread> // multithreading.

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
    std::shared_ptr<bool> isActive = std::make_shared<bool>(true);
    std::shared_ptr<TransferStatus> status = std::make_shared<TransferStatus>();

    auto run = [](std::string password, std::shared_ptr<bool> isActive, std::shared_ptr<TransferStatus> status) {
        try {
            QBXMLSync qbs("QBX", "MEPBro Sampler", "TestPassword", status, isActive); //tpass); // adding password coverage temporarily until we're ready for production values.

            //qbs.timeReset(SQLTable::inventory, 2002, 03, 13, 19, 23, 45);

            qbs.timeReset(SQLTable::inventory, 1970, 1, 1, 12, 0, 0);
            qbs.getInventory(); // get everything. 
            std::cout << "inventory complete \n";

            //qbs.updateInventoryPartnumbers(50, "ItemInventoryMod");
            std::cout << "ItemInventoryMod complete \n";

            qbs.updateInventoryPartnumbers(50, "ItemInventoryAssemblyMod");
            std::cout << "ItemInventoryAssemblyMod complete \n";

            // download full data sync.
            //if (qbs.fullsync()) {

                // testing, do a BATCH update.
            //    qbs.updateMinMaxBatch(50);
            //}

            std::cout << "Update Complete \n";
        }
        catch (ThrownError e) {
            std::cout << "Failed to load Quickbooks\n";
            SQLControl S("TestPassword");
            S.logError(e.error, e.data);
        }
    };

     //   sq.generateConnectionFile(user, pass, ip, db, xmlPass); // ("QBXML", "cVyio%90pxE4", "tcp://127.0.0.1:3306", "qbxmlstorage", "TestPassword");
   
    /*
    for (int mins{ 60 }; mins > 0; --mins) {
        for (int secs{ 60 }; secs > 0; --secs) {
            Sleep(1000);
            std::cout << "Wait Time Remaining: " << mins << ":" << secs << "\r";
        }
    }*/

    //(60 * 60 * 1000); // 1 hour, when we'll need it.

    std::thread mythread(run, "TestPassword", isActive, status);

    mythread.join();

    int i{ 0 };

    std::cin >> i;

    if (i > 0) {
        *isActive = false;
    }

    std::cin >> i;


    CoUninitialize();

    return 0;
}

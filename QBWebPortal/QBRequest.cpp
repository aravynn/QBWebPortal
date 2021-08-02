#include "QBRequest.h"

// open a connection
QBRequest::QBRequest(std::string ID, std::string appName, std::string company) : m_appID{ ID }, m_appName{ appName }, m_companyFile{ company }
{
    _bstr_t appID(m_appID.c_str());
    _bstr_t bappName(m_appName.c_str());
    _bstr_t bstrComFile(m_companyFile.c_str());
    HRESULT hr = S_OK;

    hr = pReqProc.CreateInstance(__uuidof (QBXMLRP2Lib::RequestProcessor3));
    if (SUCCEEDED(hr) && pReqProc != NULL) {
        hr = pReqProc->raw_OpenConnection(appID, bappName);

        if (SUCCEEDED(hr)) {
            hr = pReqProc->raw_BeginSession(bstrComFile, QBXMLRP2Lib::qbFileOpenDoNotCare, &m_ticket);

            if (!SUCCEEDED(hr)) {
                m_ErrorStatus = true;
                m_Msg = "ERROR: failed to create session.";
            }
            else {
                isSessionOn = true;
            }
        }
        else {
            m_ErrorStatus = true;
            m_Msg = "ERROR: Failed to Open Connection.";
        }
    }
    else {
        m_ErrorStatus = true;
        m_Msg = "ERROR: Create instance failed.";
    }
}

// close all connection process.
QBRequest::~QBRequest()
{
    pReqProc->raw_EndSession(m_ticket);
    pReqProc->raw_CloseConnection();
}

// handle the request in quickbooks. 
void QBRequest::processRequest(std::string& requestXML)
{
    if (!isSessionOn) return; // cannot process.

    _bstr_t bstrReqXML(requestXML.c_str());
    BSTR bstrResXML;
    HRESULT hr = S_OK;

    hr = pReqProc->raw_ProcessRequest(m_ticket, bstrReqXML, &bstrResXML);

    if (SUCCEEDED(hr)) {
        // convert the string to std::string
        BSTRToString(bstrResXML, m_responseXML);

       // m_responseXML = _com_util::ConvertBSTRToString(bstrResXML);
    }
    else {
        m_ErrorStatus = true;
        m_Msg = "ERROR: Failed to process message.";
    }

    ::SysFreeString(bstrResXML);
}

std::string QBRequest::getResponse()
{
    return m_responseXML;
}

void QBRequest::BSTRToString(BSTR inVal, std::string& outVal)
{
    char* tmpVal = _com_util::ConvertBSTRToString(inVal);

    if (tmpVal == NULL) {
        outVal = "";
    }
    else {
        outVal = tmpVal;
        // com_util allocates the buffer, we have to free it.
        delete[] tmpVal;
    }
}


#ifndef CDATACALLBACK_H
#define CDATACALLBACK_H

#include <wia.h>

// The application must instantiate the CDataCallback object using
// the "new" operator, and call QueryInterface to retrieve the
// IWiaDataCallback interface.
//
// In this example, using in-memory transfer, the application then
// calls the IWiaDataTransfer::idtGetBandedData method and passes
// it the IWiaDataCallback interface pointer.
//
// If the application performs a file transfer using
// IWiaDataTransfer::idtGetData, only status messages are sent,
// and the data is transferred in a file.
class CDataCallback : public IWiaDataCallback
{
private:
	LONG  m_cRef;               // Object reference count
	PBYTE m_pBuffer;            // Data buffer
	LONG  m_nBufferLength;      // Length of buffer
	LONG  m_nBytesTransfered;   // Total number of bytes transferred
	GUID  m_guidFormat;         // Data format

public:

	// Constructor and destructor
	CDataCallback();
	virtual ~CDataCallback();

	// IUnknown methods
	HRESULT CALLBACK QueryInterface(REFIID riid, void **ppvObject);
	ULONG CALLBACK AddRef();
	ULONG CALLBACK Release();

	// The IWiaDataTransfer::idtGetBandedData method periodically
	// calls the IWiaDataCallback::BandedDataCallback method with
	// status messages. It sends the callback method a data header
	// message followed by one or more data messages to transfer
	// data. It concludes by sending a termination message.
	HRESULT _stdcall BandedDataCallback(
		LONG lMessage,
		LONG lStatus,
		LONG lPercentComplete,
		LONG lOffset,
		LONG lLength,
		LONG lReserved,
		LONG lResLength,
		BYTE *pbData);
};

#endif // CDATACALLBACK_H
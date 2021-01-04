#include "framework.h"
#include "cdatacallback.h"
//#include "amslogger.h"

CDataCallback::CDataCallback() :
	m_cRef(1),
	m_pBuffer(NULL),
	m_nBufferLength(0),
	m_nBytesTransfered(0),
	m_guidFormat(IID_NULL)
{
}

CDataCallback::~CDataCallback()
{
	// Free the item buffer
	if (m_pBuffer)
	{
		LocalFree(m_pBuffer);
		m_pBuffer = NULL;
	}
	m_nBufferLength = 0;
	m_nBytesTransfered = 0;
}

HRESULT CDataCallback::QueryInterface(REFIID riid, void **ppvObject)
{
	// Validate arguments
	if (NULL == ppvObject)
	{
		return E_INVALIDARG;
	}

	// Return the appropriate interface
	if (IsEqualIID(riid, IID_IUnknown))
	{
		*ppvObject = static_cast<CDataCallback *>(this);
	}
	else if (IsEqualIID(riid, IID_IWiaDataCallback))
	{
		*ppvObject = static_cast<CDataCallback *>(this);
	}
	else
	{
		*ppvObject = NULL;
		return(E_NOINTERFACE);
	}

	// Increment the reference count before returning the interface
	reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
	return S_OK;
}

ULONG CDataCallback::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG CDataCallback::Release()
{
	LONG cRef = InterlockedDecrement(&m_cRef);
	if (0 == cRef)
	{
		delete this;
	}
	return cRef;
}

HRESULT CDataCallback::BandedDataCallback(LONG lMessage, LONG lStatus,
	LONG lPercentComplete, LONG lOffset,
	LONG lLength, LONG lReserved,
	LONG lResLength, BYTE *pbData)
{
	UNREFERENCED_PARAMETER(lReserved);
	UNREFERENCED_PARAMETER(lResLength);
	switch (lMessage)
	{
	case IT_MSG_DATA_HEADER:
	{
		// The data header contains the image's final size.
		PWIA_DATA_CALLBACK_HEADER pHeader =
			reinterpret_cast<PWIA_DATA_CALLBACK_HEADER>(pbData);
		if (pHeader && pHeader->lBufferSize)
		{
			// Allocate a block of memory to hold the image
			m_pBuffer = reinterpret_cast<PBYTE>(
				LocalAlloc(LPTR, pHeader->lBufferSize));
			if (m_pBuffer)
			{
				// Save the buffer size.
				m_nBufferLength = pHeader->lBufferSize;

				// Initialize the bytes transferred count
				m_nBytesTransfered = 0;

				// Save the file format.
				m_guidFormat = pHeader->guidFormatID;
			}
		}
	}
	break;

	case IT_MSG_DATA:
	{
		// Make sure a block of memory has been created.
		if (NULL != m_pBuffer)
		{
			// Copy the new band.
			CopyMemory(m_pBuffer + lOffset, pbData, lLength);

			// Increment the byte count
			m_nBytesTransfered += lLength;
		}
	}
	break;

	case IT_MSG_STATUS:
	{
		// Display transfer phase
		if (lStatus & IT_STATUS_TRANSFER_FROM_DEVICE)
		{
			//LogDebug() << "Transfer from device\n";
		}
		else if (lStatus & IT_STATUS_PROCESSING_DATA)
		{
			//LogDebug() << "Processing Data\n";
		}
		else if (lStatus & IT_STATUS_TRANSFER_TO_CLIENT)
		{
			//LogDebug() << "Transfer to Client\n";
		}

		// Display percent complete
		//LogDebug() << "lPercentComplete: " << (int)lPercentComplete;
	}
	break;
	}

	return S_OK;
}
// WindowsProject1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowsProject2.h"
#include "wia.h"
#include "cdatacallback.h"

#include <iostream>
#include <fstream>
using namespace std;
#pragma comment(lib,"Ole32.lib")  // linker issues
#pragma comment(lib,"wiaguid.lib")  // linker issues

#define MAX_LOADSTRING 100
#define BUTTON_ID      1001
#define ENUM_BUTTON_ID     1002
//#define APP_SCAN 102
//#define APP_SAVE 103

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
IWiaDevMgr2* pWiaDevMgr2 = NULL;
BSTR bstrDeviceID;
IWiaItem2 **ppWiaDevice = NULL;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
IWiaDevMgr2* ObtainWiaInterface(const HWND& hWnd, int& retflag);
void ReleaseWiaInterface();
HRESULT EnumerateWiaDevices(IWiaDevMgr2* pWiaDevMgr);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void ReportError(LPCTSTR pszMessage, HRESULT hr = S_OK);

HRESULT CreateWiaDevice(IWiaDevMgr2 *pWiaDevMgr, BSTR bstrDeviceID,
	IWiaItem2 **ppWiaDevice);
HRESULT enumerateItems(IWiaItem2 *pWiaItem);
HRESULT transferWiaItem(IWiaItem2 *pWiaItem);



IWiaDevMgr2* ObtainWiaInterface(const HWND& hWnd, int& retflag)
{
	retflag = 1;

	if (NULL == pWiaDevMgr2)
	{
		HRESULT hr;
		hr = CoCreateInstance(CLSID_WiaDevMgr2,
			NULL,
			CLSCTX_LOCAL_SERVER,
			IID_IWiaDevMgr2,
			(LPVOID*)&pWiaDevMgr2);

		if (FAILED(hr))
		{
			MessageBox(hWnd, TEXT("Failed CoCreateInstance WiaDevMgr [%x]\n"), TEXT("Erroor"), hr);
			{ retflag = 2; return NULL; };
		}
	}

	return pWiaDevMgr2;
}

void ReleaseWiaInterface()
{
	if (NULL != pWiaDevMgr2)
	{
		pWiaDevMgr2->Release();
		pWiaDevMgr2 = NULL;
	}
}



void ReportError(LPCTSTR pszMessage, HRESULT hr)
{
	if (S_OK != hr)
	{
		_tprintf(TEXT("%s: HRESULT: 0x%08X\n"), pszMessage, hr);
	}
	else
	{
		_tprintf(TEXT("%s\n"), pszMessage);
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WINDOWSPROJECT2, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT2));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT2));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT2);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}



//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	static HWND hButton;
	static HWND hEnumButton;

	POINT pt;

	switch (message)
	{
	case WM_CREATE:
		hButton = CreateWindow(L"button", L"Label",
			WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			100, 200,
			50, 20,
			hWnd, (HMENU)BUTTON_ID,
			hInst, NULL);
		hEnumButton = CreateWindow(L"button", L"Enum Button",
			WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			100, 270,
			150, 20,
			hWnd, (HMENU)ENUM_BUTTON_ID,
			hInst, NULL);

		break;
	case WM_COMMAND:
	{
		int retflag;
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case BUTTON_ID:
			//MessageBox(hWnd, TEXT(""), TEXT(""), 0);


			ObtainWiaInterface(hWnd, retflag);
			if (retflag == 2) break;
			break;

		case ENUM_BUTTON_ID:
			ObtainWiaInterface(hWnd, retflag);
			if (retflag == 2) break;

			EnumerateWiaDevices(pWiaDevMgr2);

			CreateWiaDevice(pWiaDevMgr2, bstrDeviceID, ppWiaDevice);

			ReleaseWiaInterface();


			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

HRESULT CreateWiaDevice( IWiaDevMgr2 *pWiaDevMgr, BSTR bstrDeviceID,
                               IWiaItem2 **ppWiaDevice )
{
  // Validate arguments
  if (NULL == pWiaDevMgr || NULL == bstrDeviceID || NULL == ppWiaDevice)
  {
    return E_INVALIDARG;
  }

  // Initialize out variables
  *ppWiaDevice = NULL;

  // Create the WIA Device
  HRESULT hr = pWiaDevMgr->CreateDevice(0, bstrDeviceID, ppWiaDevice );

  // Return the result of creating the device
  return hr;
}



HRESULT enumerateItems(IWiaItem2 *pWiaItem)
{
  // Validate arguments
  if (NULL == pWiaItem)
  {
    return E_INVALIDARG;
  }

  // Get the item type for this item.
  LONG lItemType = 0;
  HRESULT hr = pWiaItem->GetItemType( &lItemType );
  if (SUCCEEDED(hr))
  {
    // If it is a folder, or it has attachments, enumerate its children.
    if (lItemType & WiaItemTypeFolder || lItemType & WiaItemTypeHasAttachments)
    {
      // Get the child item enumerator for this item.
      IEnumWiaItem2 *pEnumWiaItem = NULL;
      hr = pWiaItem->EnumChildItems(NULL,  &pEnumWiaItem );
      if (SUCCEEDED(hr))
      {
        // Loop until you get an error or pEnumWiaItem->Next returns
        // S_FALSE to signal the end of the list.
        while (S_OK == hr)
        {
          // Get the next child item.
          IWiaItem2 *pChildWiaItem = NULL;
          hr = pEnumWiaItem->Next( 1, &pChildWiaItem, NULL );

          // pEnumWiaItem->Next will return S_FALSE when the list is
          // exhausted, so check for S_OK before using the returned
          // value.
          if (S_OK == hr)
          {
            // Recurse into this item.
            hr = enumerateItems( pChildWiaItem );

            // Release this item.
            pChildWiaItem->Release();
            pChildWiaItem = NULL;
          }
        }

        // If the result of the enumeration is S_FALSE (which
        // is normal), change it to S_OK.
        if (S_FALSE == hr)
        {
          hr = S_OK;
        }

        // Release the enumerator.
        pEnumWiaItem->Release();
        pEnumWiaItem = NULL;
      }
    }
  }
  return  hr;
}

HRESULT transferWiaItem(IWiaItem2 *pWiaItem)
{
  // Validate arguments
  if (NULL == pWiaItem)
  {
    return E_INVALIDARG;
  }

  // Get the IWiaPropertyStorage interface so you can set required properties.
  IWiaPropertyStorage *pWiaPropertyStorage = NULL;
  HRESULT hr = pWiaItem->QueryInterface( IID_IWiaPropertyStorage,
                                         (void**)&pWiaPropertyStorage );
  if (SUCCEEDED(hr))
  {
    // Prepare PROPSPECs and PROPVARIANTs for setting the
    // media type and format
    PROPSPEC PropSpec[2] = {0, 0};
    PROPVARIANT PropVariant[2] = {0, 0, 0, 0, 0};
    const ULONG c_nPropCount = sizeof(PropVariant)/sizeof(PropVariant[0]);

    // Use BMP as the output format
    GUID guidOutputFormat = WiaImgFmt_BMP;

    // Initialize the PROPSPECs
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = WIA_IPA_FORMAT;
    PropSpec[1].ulKind = PRSPEC_PROPID;
    PropSpec[1].propid = WIA_IPA_TYMED;

    // Initialize the PROPVARIANTs
    PropVariant[0].vt = VT_CLSID;
    PropVariant[0].puuid = &guidOutputFormat;
    PropVariant[1].vt = VT_I4;
    PropVariant[1].lVal = TYMED_FILE;

    // Set the properties
    hr = pWiaPropertyStorage->WriteMultiple( c_nPropCount, PropSpec,
                                             PropVariant, WIA_IPA_FIRST );
    if (SUCCEEDED(hr))
    {
      // Get the IWiaDataTransfer interface
      IWiaDataTransfer *pWiaDataTransfer = NULL;
      hr = pWiaItem->QueryInterface( IID_IWiaDataTransfer,
                                     (void**)&pWiaDataTransfer );
      if (SUCCEEDED(hr))
      {
        // Create our callback class

        CDataCallback *pCallback = new CDataCallback;
        if (pCallback)

        {
          // Get the IWiaDataCallback interface from our callback class.
          
		  IWiaDataCallback *pWiaDataCallback = NULL;
          hr = pCallback->QueryInterface( IID_IWiaDataCallback,
                                          (void**)&pWiaDataCallback );
          if (SUCCEEDED(hr))

          {
            // Perform the transfer using default settings
            STGMEDIUM stgMedium = {0, 0, NULL};
            stgMedium.tymed = TYMED_FILE;
            hr = pWiaDataTransfer->idtGetData( &stgMedium, pWiaDataCallback );
            if (S_OK == hr)
            {
              // Print the filename (note that this filename is always
              // a WCHAR string, not TCHAR).


				ofstream myfile;
				myfile.open(stgMedium.lpszFileName);
				//myfile << 

              /*QString fileName = QString::fromWCharArray(stgMedium.lpszFileName);
              LogDebug()<<"Transferred filename: "<<fileName;
              QImage img(fileName);
              QTemporaryFile file(qApp->applicationDirPath()+"/scanXXXXXX.png");
              file.setAutoRemove(false);
              img.save(&file, "PNG");*/

              // Release any memory associated with the stgmedium
              // This will delete the file stgMedium.lpszFileName.
              ReleaseStgMedium( &stgMedium );
            }

            // Release the callback interface
            pWiaDataCallback->Release();
            pWiaDataCallback = NULL;
          }

          // Release our callback.  It should now delete itself.
          pCallback->Release();
          pCallback = NULL;
        }

        // Release the IWiaDataTransfer
        pWiaDataTransfer->Release();
        pWiaDataTransfer = NULL;
      }
    }

    // Release the IWiaPropertyStorage
    pWiaPropertyStorage->Release();
    pWiaPropertyStorage = NULL;
  }

  return hr;
}


HRESULT EnumerateWiaDevices(IWiaDevMgr2* pWiaDevMgr) //Vista or later

{
	//
	// Validate arguments
	//
	if (NULL == pWiaDevMgr)
	{
		return E_INVALIDARG;
	}

	//
	// Get a device enumerator interface
	//
	IEnumWIA_DEV_INFO* pWiaEnumDevInfo = NULL;
	HRESULT hr = pWiaDevMgr->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL, &pWiaEnumDevInfo);
	if (SUCCEEDED(hr))
	{
		//
		// Loop until you get an error or pWiaEnumDevInfo->Next returns
		// S_FALSE to signal the end of the list.
		//
		while (S_OK == hr)
		{
			//
			// Get the next device's property storage interface pointer
			//
			IWiaPropertyStorage* pWiaPropertyStorage = NULL;
			hr = pWiaEnumDevInfo->Next(1, &pWiaPropertyStorage, NULL);

			//
			// pWiaEnumDevInfo->Next will return S_FALSE when the list is
			// exhausted, so check for S_OK before using the returned
			// value.
			//
			if (hr == S_OK)
			{
				//
				// Do something with the device's IWiaPropertyStorage*
				//

				PROPSPEC PropSpec[3] = { 0 };
				PROPVARIANT PropVar[3] = { 0 };

				//
				// How many properties are we querying for?
				//
				const ULONG c_nPropertyCount = sizeof(PropSpec) / sizeof(PropSpec[0]);

				//
				// Define which properties we want to read:
				// Device ID.  This is what we'd use to create
				// the device.
				//
				PropSpec[0].ulKind = PRSPEC_PROPID;
				PropSpec[0].propid = WIA_DIP_DEV_ID; //WIA_IPA_FULL_ITEM_NAME
				PropSpec[1].ulKind = PRSPEC_PROPID;
				PropSpec[1].propid = WIA_DIP_DEV_NAME; //WIA_IPA_ITEM_NAME
				PropSpec[2].ulKind = PRSPEC_PROPID;
				PropSpec[2].propid = WIA_DIP_DEV_DESC; //WIA_DPS_DEVICE_ID
						

				//
				// Ask for the property values
				//
				hr = pWiaPropertyStorage->ReadMultiple(c_nPropertyCount, PropSpec, PropVar);
				if (SUCCEEDED(hr))
				{
					//
					// IWiaPropertyStorage::ReadMultiple will return S_FALSE if some
					// properties could not be read, so we have to check the return
					// types for each requested item.
					//

					//
					// Check the return type for the device ID
					//
					if (VT_BSTR == PropVar[0].vt)
					{
						//
						// Do something with the device ID
						//
						bstrDeviceID = PropVar[0].bstrVal;
						_tprintf(TEXT("Item Name: %ws\n"), PropVar[0].bstrVal);
					}

					//
					// Free the returned PROPVARIANTs
					//
					FreePropVariantArray(c_nPropertyCount, PropVar);
				}
				else
				{
					ReportError(TEXT("Error calling IWiaPropertyStorage::ReadMultiple"), hr);
				}

				//
				// Release the device's IWiaPropertyStorage*
				//
				pWiaPropertyStorage->Release();
				pWiaPropertyStorage = NULL;
			}
		}

		//
		// If the result of the enumeration is S_FALSE (which
		// is normal), change it to S_OK.
		//
		if (S_FALSE == hr)
		{
			hr = S_OK;
		}

		//
		// Release the enumerator
		//
		pWiaEnumDevInfo->Release();
		pWiaEnumDevInfo = NULL;
	}

	//
	// Return the result of the enumeration
	//
	return hr;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

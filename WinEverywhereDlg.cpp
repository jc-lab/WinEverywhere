
// WinEverywhereDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "WinEverywhere.h"
#include "WinEverywhereDlg.h"
#include "afxdialogex.h"

#include <initguid.h>
#include <shldisp.h>
#include <shlguid.h>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <mutex>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWinEverywhereDlg 대화 상자

struct WindowInfo {
	HWND handle_;
	std::basic_string<TCHAR> title_;

	WindowInfo(HWND hWnd, LPCTSTR title) : handle_(hWnd), title_(title) {
	}
};

class CWinEverywhereDlg::MyEnumString : public IEnumString {
private:
	LONG m_cRef;
	int m_nCurrentElement;

public:
	std::shared_ptr<MyEnumString> sp_;

	std::mutex mutex_;
	std::list<WindowInfo> list_;

	MyEnumString() : m_cRef(0), m_nCurrentElement(0) {
	}

	//	IUnknown Interface
	ULONG STDMETHODCALLTYPE AddRef() override {
		return ::InterlockedIncrement(&m_cRef);
		return S_OK;
	}
	ULONG STDMETHODCALLTYPE Release() override {
		ULONG nCount = 0;

		nCount = (ULONG) ::InterlockedDecrement(&m_cRef);

		if (nCount == 0) {
			sp_.reset();
		}

		return nCount;
	}
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {

		HRESULT hr = E_NOINTERFACE;
		if (ppvObject != NULL)
		{
			*ppvObject = NULL;

			if (IID_IUnknown == riid)
				* ppvObject = static_cast<IUnknown*>(this);

			else if (IID_IEnumString == riid)
				* ppvObject = static_cast<IEnumString*>(this);

			if (*ppvObject != NULL)
			{
				hr = S_OK;
				((LPUNKNOWN)* ppvObject)->AddRef();
			}
		} else {
			hr = E_POINTER;
		}

		return hr;
	}
	// IEnumString Interface
	HRESULT STDMETHODCALLTYPE Clone(IEnumString** ppenum) override {
		return E_NOTIMPL;
	}
	HRESULT STDMETHODCALLTYPE Next(ULONG celt, LPOLESTR* rgelt, ULONG* pceltFetched) override {
		std::unique_lock<std::mutex> lock(mutex_);

		HRESULT hr = S_FALSE;

		if (0 == celt)
			celt = 1;

		ULONG i = 0;
		for (i = 0; i < celt; i++)
		{
			if (m_nCurrentElement == (ULONG)list_.size())
				break;

			auto it = list_.begin();
			for (ULONG idx = 0; idx < m_nCurrentElement; idx++)
				++it;

			size_t buf_count = (it->title_.size() + 1);
			rgelt[i] = (LPWSTR)::CoTaskMemAlloc((ULONG)sizeof(wchar_t) * buf_count);
			wcscpy_s(rgelt[i], buf_count, it->title_.c_str());

			if (pceltFetched)
				* pceltFetched++;

			m_nCurrentElement++;
		}

		if (i == celt)
			hr = S_OK;

		return hr;
	}
	HRESULT STDMETHODCALLTYPE Reset() override {
		m_nCurrentElement = 0;

		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE Skip(ULONG celt) override {
		std::unique_lock<std::mutex> lock(mutex_);

		m_nCurrentElement += celt;

		if (m_nCurrentElement > (ULONG)list_.size())
			m_nCurrentElement = 0;
		
		return S_OK;
	}
};

CWinEverywhereDlg::CWinEverywhereDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SEARCH_WINDOW, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWinEverywhereDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_INPUT_TEXT, input_text_edit_);
}

BEGIN_MESSAGE_MAP(CWinEverywhereDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CWinEverywhereDlg 메시지 처리기

BOOL CWinEverywhereDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	my_enum_string_.reset(new MyEnumString());
	my_enum_string_->sp_ = my_enum_string_;

	{
		HRESULT hr = m_pac.CoCreateInstance(CLSID_AutoComplete);
		if (SUCCEEDED(hr)) {
			CComQIPtr<IAutoComplete2> pAC2(m_pac);
			hr = pAC2->SetOptions(ACO_UPDOWNKEYDROPSLIST | ACO_AUTOSUGGEST | ACO_AUTOAPPEND | ACO_SEARCH);
			pAC2.Release();

			hr = m_pac->Init(input_text_edit_.GetSafeHwnd(), my_enum_string_.get(), NULL, NULL);
		}
	}

	updateWindowList();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CWinEverywhereDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CWinEverywhereDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




static BOOL CALLBACK EnumWindowsProc(
	_In_ HWND   hwnd,
	_In_ LPARAM lParam
)
{
	std::list<WindowInfo>* plist = (std::list<WindowInfo>*)lParam;
	TCHAR szName[128];
	GetWindowText(hwnd, szName, 128);
	if (_tcslen(szName) > 0)
	{
		plist->push_back(WindowInfo(hwnd, szName));
	}
	return TRUE;
}

void CWinEverywhereDlg::updateWindowList() {
	std::unique_lock<std::mutex> lock(my_enum_string_->mutex_);
	my_enum_string_->list_.clear();
	EnumWindows(EnumWindowsProc, (LPARAM)& my_enum_string_->list_);
}

BOOL CWinEverywhereDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE) {
			this->ShowWindow(SW_HIDE);
			return TRUE;
		}else
		if (pMsg->wParam == VK_RETURN) {
			if (GetDlgItem(IDC_INPUT_TEXT) == GetFocus())
			{
				std::unique_lock<std::mutex> lock(my_enum_string_->mutex_);
				CString text;
				bool found = false;
				input_text_edit_.GetWindowText(text);
				for (auto it = my_enum_string_->list_.cbegin(); it != my_enum_string_->list_.cend(); it++) {
					if (it->title_.size() >= text.GetLength() && it->title_.compare(0, text.GetLength(), text, text.GetLength()) == 0) {
						::SetForegroundWindow(it->handle_);
						_tprintf(_T("Found Window : %s\n"), it->title_.c_str());
						found = true;
					}
				}
				if (found) {
					this->ShowWindow(SW_HIDE);
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CWinEverywhereDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (bShow) {
		updateWindowList();
		input_text_edit_.SetWindowText(_T(""));
		input_text_edit_.SetFocus();
	}
}

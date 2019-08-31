
// WinEverywhere.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "WinEverywhere.h"
#include "WinEverywhereDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWinEverywhereApp

BEGIN_MESSAGE_MAP(CWinEverywhereApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWinEverywhereApp 생성

CWinEverywhereApp::CWinEverywhereApp()
{
	// 다시 시작 관리자 지원
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CWinEverywhereApp 개체입니다.

CWinEverywhereApp theApp;
HHOOK hook_;

int64_t old_time_ = 0;

// This is the callback function. Consider it the event that is raised when, in this case, 
// a key is pressed.
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT *kbdStruct;

	if (nCode >= 0)
	{
		if (wParam == WM_SYSKEYDOWN)
		{
			// lParam is the pointer to the struct containing the data needed, so cast and assign it to kdbStruct.
			kbdStruct = ((KBDLLHOOKSTRUCT*)lParam);

			// a key (non-system) is pressed.
			if (kbdStruct->vkCode == VK_LMENU)
			{
				int64_t cur = _time64(NULL);

				if (old_time_ == 0) {
					old_time_ = _time64(NULL);
				}
				else if((cur - old_time_) < 500) {

					theApp.m_pMainWnd->ShowWindow(SW_SHOW);
					theApp.m_pMainWnd->SetForegroundWindow();
					old_time_ = 0;
					return 0;
				}
			}
		}
	}

	// call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
	return CallNextHookEx(hook_, nCode, wParam, lParam);
}


// CWinEverywhereApp 초기화

BOOL CWinEverywhereApp::InitInstance()
{
	// 애플리케이션 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// 대화 상자에 셸 트리 뷰 또는
	// 셸 목록 뷰 컨트롤이 포함되어 있는 경우 셸 관리자를 만듭니다.
	CShellManager *pShellManager = new CShellManager;

	// MFC 컨트롤의 테마를 사용하기 위해 "Windows 원형" 비주얼 관리자 활성화
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 애플리케이션 마법사에서 생성된 애플리케이션"));

	CoInitialize(NULL);

	CWinEverywhereDlg dlg;
	m_pMainWnd = &dlg;

#ifdef _DEBUG
	AllocConsole();

	FILE* con_stdin = NULL;
	FILE* con_stdout = NULL;
	FILE* con_stderr = NULL;

	freopen_s(&con_stdin, "CONIN$", "r", stdin);
	freopen_s(&con_stdout, "CONOUT$", "w", stdout);
	freopen_s(&con_stderr, "CONOUT$", "w", stderr);
#endif

	hook_ = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0);
	if (!hook_) {
		MessageBox(NULL, _T("SetWindowsHookEx Failed"), _T("WinEverywhere"), MB_OK);
		return 1;
	}

	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 여기에 [취소]를 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "경고: 대화 상자를 만들지 못했으므로 애플리케이션이 예기치 않게 종료됩니다.\n");
		TRACE(traceAppMsg, 0, "경고: 대화 상자에서 MFC 컨트롤을 사용하는 경우 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS를 수행할 수 없습니다.\n");
	}

	UnhookWindowsHookEx(hook_);

	::CoUninitialize();

	// 위에서 만든 셸 관리자를 삭제합니다.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고  응용 프로그램을 끝낼 수 있도록 FALSE를
	// 반환합니다.
	return FALSE;
}


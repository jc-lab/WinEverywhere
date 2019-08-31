
// WinEverywhereDlg.h: 헤더 파일
//

#pragma once

#include "Resource.h"

#include <memory>

// CWinEverywhereDlg 대화 상자
class CWinEverywhereDlg : public CDialogEx
{
// 생성입니다.
public:
	class MyEnumString;

	CWinEverywhereDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SEARCH_WINDOW };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

// 구현입니다.
protected:
	HICON m_hIcon;

	std::shared_ptr<MyEnumString> my_enum_string_;
	CComPtr<IAutoComplete> m_pac;

	CEdit input_text_edit_;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void updateWindowList();
	BOOL PreTranslateMessage(MSG* pMsg) override;

	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

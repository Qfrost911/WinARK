#include "stdafx.h"
#include "resource.h"
#include "KernelHookView.h"

LRESULT CKernelHookView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	CRect r(0, 0, 400, 25);
	CTabCtrl tabCtrl;
	auto hTabCtrl = tabCtrl.Create(m_hWnd, &r, nullptr, WS_CHILDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS
		| TCS_HOTTRACK | TCS_SINGLELINE | TCS_RIGHTJUSTIFY | TCS_TABS,
		WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY,TabId);
	m_TabCtrl.SubclassWindow(hTabCtrl);
	
	//m_TabCtrl.SetFont()
	HFONT hFont = (HFONT)::GetStockObject(SYSTEM_FIXED_FONT);
	m_TabCtrl.SetFont(hFont, true);
	::DeleteObject(hFont);

	struct {
		PCWSTR Name;
	}columns[] = {
		L"SSDT",
		L"Shadow SSDT",
		L"Kernel Notifications"
	};

	int i = 0;
	for (auto& col : columns) {
		m_TabCtrl.InsertItem(i++, col.Name);
	}

	InitSSDTHookTable();
	InitShadowSSDTHookTable();
	InitKernelNotifyTable();




	return 0;
}

LRESULT CKernelHookView::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	CRect rc;
	GetClientRect(&rc);
	int width = rc.Width();
	int clientHeight = rc.Height();
	m_TabCtrl.GetClientRect(&rc);
	int height = rc.Height();
	::MoveWindow(m_TabCtrl.m_hWnd, rc.left, rc.top, width, height, true);

	int iX = rc.left;
	int iY = rc.top + height;
	clientHeight -= height;
	
	::MoveWindow(m_hwndArray[_index], iX, iY, width, clientHeight, true);

	bHandled = false;
	return 0;
}

void CKernelHookView::InitSSDTHookTable() {
	BarDesc bars[] = {
		{12,"服务号",0},
		{55,"服务函数名",0},
		{20,"原始函数地址",0},
		{10,"是否Hook",0},
		{20,"当前函数地址",0},
		{260,"当前函数地址所在模块",0}
	};

	TableInfo table = {
		1,1,TABLE_SORTMENU | TABLE_COPYMENU | TABLE_APPMENU,9,0,0,0
	};

	BarInfo info;
	info.nbar = _countof(bars);
	info.font = 9;
	for (int i = 0; i < info.nbar; i++) {
		info.bar[i].defdx = bars[i].defdx;
		info.bar[i].mode = bars[i].mode;
		info.bar[i].name = bars[i].name;
	}

	m_SSDTHookTable = new CSSDTHookTable(info, table);
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	m_SSDTHookTable->Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_EX_LAYERED);
	m_hwndArray[static_cast<int>(TabColumn::SSDT)] = m_SSDTHookTable->m_hWnd;
	m_SSDTHookTable->ShowWindow(SW_SHOW);
}

void CKernelHookView::InitShadowSSDTHookTable() {
	BarDesc bars[] = {
		{15,"服务号",0},
		{55,"服务函数名",0},
		{20,"原始函数地址",0},
		{10,"是否Hook",0},
		{20,"当前函数地址",0},
		{260,"当前函数地址所在模块",0}
	};

	TableInfo table = {
		1,1,TABLE_SORTMENU | TABLE_COPYMENU | TABLE_APPMENU,9,0,0,0
	};

	BarInfo info;
	info.nbar = _countof(bars);
	info.font = 9;
	for (int i = 0; i < info.nbar; i++) {
		info.bar[i].defdx = bars[i].defdx;
		info.bar[i].mode = bars[i].mode;
		info.bar[i].name = bars[i].name;
	}

	m_ShadowSSDTHookTable = new CShadowSSDTHookTable(info, table);
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	m_ShadowSSDTHookTable->Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_EX_LAYERED);
	m_hwndArray[static_cast<int>(TabColumn::ShadowSSDT)] = m_ShadowSSDTHookTable->m_hWnd;
	m_ShadowSSDTHookTable->ShowWindow(SW_HIDE);
}


LRESULT CKernelHookView::OnTcnSelChange(int, LPNMHDR hdr, BOOL&) {
	int index = 0;
	
	index = m_TabCtrl.GetCurSel();
	m_SSDTHookTable->ShowWindow(SW_HIDE);
	m_ShadowSSDTHookTable->ShowWindow(SW_HIDE);
	m_KernelNotifyTable->ShowWindow(SW_HIDE);

	switch (static_cast<TabColumn>(index)) {
		case TabColumn::SSDT:
			m_SSDTHookTable->ShowWindow(SW_SHOW);
			m_SSDTHookTable->SetFocus();
			break;
		case TabColumn::ShadowSSDT:
			m_ShadowSSDTHookTable->ShowWindow(SW_SHOW);
			m_ShadowSSDTHookTable->SetFocus();
			break;
		case TabColumn::ObjectCallback:
			m_KernelNotifyTable->ShowWindow(SW_SHOW);
			m_KernelNotifyTable->SetFocus();
			break;
	}
	_index = index;
	::PostMessage(m_hWnd, WM_SIZE, 0, 0);
	return 0;
}

void CKernelHookView::InitKernelNotifyTable() {
	BarDesc bars[] = {
		{20,"回调函数地址",0},
		{25,"回调类型",0},
		{25,"文件厂商",0},
		{115,"所在模块",0},
	};

	TableInfo table = {
		1,1,TABLE_SORTMENU | TABLE_COPYMENU | TABLE_APPMENU,9,0,0,0
	};

	BarInfo info;
	info.nbar = _countof(bars);
	info.font = 9;
	for (int i = 0; i < info.nbar; i++) {
		info.bar[i].defdx = bars[i].defdx;
		info.bar[i].mode = bars[i].mode;
		info.bar[i].name = bars[i].name;
	}

	m_KernelNotifyTable = new CKernelNotifyTable(info, table);
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	m_KernelNotifyTable->Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_EX_LAYERED);
	m_hwndArray[static_cast<int>(TabColumn::ObjectCallback)] = m_KernelNotifyTable->m_hWnd;
	m_KernelNotifyTable->ShowWindow(SW_HIDE);
}
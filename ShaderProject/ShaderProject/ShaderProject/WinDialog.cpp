#include "WinDialog.h"
#include <Windows.h>
#include "StackTrace.h"
#include "Argument.h"
#include <sstream>

const long long CONTROL_ID_TEXT = 1001;
const long long CONTROL_ID_BUTTON = 1002;
const unsigned int CONTROL_TEXT_LEN = 256;

DialogTextReason g_dialogTextReason = DIALOG_TEXT_INITIALIZE;
char g_controlText[CONTROL_TEXT_LEN];

LRESULT CALLBACK DialogTextWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void DialogMessage(unsigned char stacktrace, const char* text, ...)
{
	// �X�^�b�N�g���[�X���擾
	StackTrace stackTrace = GetStackTrace();

	// �t�H�[�}�b�g��������擾
	VA_TEXT(ptr, text);

	// �\�����b�Z�[�W
	std::string str;
	std::stringstream ss;
	ss << stackTrace[stacktrace].filePath << std::endl;
	ss << stackTrace[stacktrace].fileName << "(" << stackTrace[stacktrace].line << ")" << std::endl;
	ss << "----------------" << std::endl;
	ss << ptr << std::endl;
	ss << "----------------" << std::endl;
	str = ss.str();

	// ���b�Z�[�W�{�b�N�X�̐ݒ�
	MSGBOXPARAMS mbp = {};
	mbp.cbSize = sizeof(mbp);
	mbp.hwndOwner = nullptr;
	mbp.lpszCaption = "Warning";
	mbp.lpszText = str.c_str();
	mbp.dwStyle = MB_OK | MB_ICONWARNING;
	MessageBoxIndirect(&mbp);
}

void DialogError(unsigned char stacktrace, const char* text, ...)
{
	// �X�^�b�N�g���[�X���擾
	StackTrace stackTrace = GetStackTrace();

	// �t�H�[�}�b�g��������擾
	VA_TEXT(ptr, text);

	// �\�����b�Z�[�W
	std::string str;
	std::stringstream ss;
	ss << stackTrace[stacktrace].filePath << std::endl;
	ss << stackTrace[stacktrace].fileName << "(" << stackTrace[stacktrace].line << ")" << std::endl;
	ss << "----------------" << std::endl;
	ss << ptr << std::endl;
	ss << "----------------" << std::endl;
	ss << "[���@�~] �v���O�������I��" << std::endl;
	ss << "[�Ď��s] �ڍׂȃX�^�b�N��\��" << std::endl;
	ss << "[���@��] �v���O�����𑱍s" << std::endl;
	str = ss.str();

	// ���b�Z�[�W�{�b�N�X�̐ݒ�
	MSGBOXPARAMS mbp = {};
	mbp.cbSize = sizeof(mbp);
	mbp.hwndOwner = nullptr;
	mbp.lpszCaption = "Error";
	mbp.lpszText = str.c_str();
	mbp.dwStyle = MB_ABORTRETRYIGNORE | MB_ICONERROR;

	// ���b�Z�[�W�{�b�N�X�\��
	switch (MessageBoxIndirect(&mbp))
	{
	case IDRETRY:
		ss.str("");
		for (int i = stacktrace; i < stackTrace.size(); ++i)
		{
			ss << "[" << i << "]" << std::endl;
			ss << stackTrace[i].filePath << std::endl;
			ss << stackTrace[i].fileName;
			ss << "(" << stackTrace[i].line << ") : ";
			ss << stackTrace[i].funcName;
			ss << std::endl;
		}
		str = ss.str();
		mbp.lpszText = str.c_str();
		mbp.dwStyle = MB_ICONERROR | MB_OK;
		MessageBoxIndirect(&mbp);
		// fall throught
	case IDABORT:
		// �I��
		PostQuitMessage(0);
		break;
	case IDIGNORE:
		// ����
		break;
	}
}

DialogTextReason DialogText(char* pOut, int outSize)
{
	static const char* DialogClassName = "DialogText";
	static const DWORD style = WS_CAPTION | WS_SYSMENU;
	static const DWORD exStyle = WS_EX_OVERLAPPEDWINDOW;
	static const int DialogWindowWidth		= 350;
	static const int DialogWindowHeight		= 50;
	static const int DialogWindowMargin		= 10;
	static const int DialogButtonWidth		= 40;
	static const int DialogControlMargin	= 5;
	static const int DialogControlHeight	= DialogWindowHeight - DialogWindowMargin * 2;
	static const int DialogControlY			= DialogWindowMargin;
	static const int DialogTextX			= DialogWindowMargin;
	static const int DialogTextWidth		= DialogWindowWidth - (DialogWindowMargin * 2 + DialogButtonWidth + DialogControlMargin);
	static const int DialogButtonX			= DialogWindowWidth - (DialogWindowMargin + DialogButtonWidth);

	static RECT rect = {0, 0, DialogWindowWidth, DialogWindowHeight};
	static HINSTANCE hInstance = GetModuleHandle(NULL);

	switch (g_dialogTextReason)
	{
	case DIALOG_TEXT_FAILED:
	case DIALOG_TEXT_INITIALIZE:
	{
		// �E�B���h�N���X���̐ݒ�
		WNDCLASSEX wcex;
		ZeroMemory(&wcex, sizeof(wcex));
		wcex.hInstance = hInstance;
		wcex.lpszClassName = DialogClassName;
		wcex.lpfnWndProc = DialogTextWndProc;
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hIconSm = wcex.hIcon;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

		// �E�B���h�E�N���X���̓o�^
		if (!RegisterClassEx(&wcex))
		{
			MessageBox(NULL, "Failed to RegisterClassEx", "Error", MB_OK);
			g_dialogTextReason = DIALOG_TEXT_FAILED;
			return g_dialogTextReason;
		}

		// �E�B���h�E�T�C�Y�v�Z
		AdjustWindowRectEx(&rect, style, false, exStyle);
		g_dialogTextReason = DIALOG_TEXT_CREATE;
		// fall throught
	}

	case DIALOG_TEXT_CLOSE:
	case DIALOG_TEXT_CREATE:
	{
		HWND hWnd;
		hWnd = CreateWindowEx(exStyle, DialogClassName,
			"�e�L�X�g�����", style,
			CW_USEDEFAULT, CW_USEDEFAULT,
			rect.right - rect.left, rect.bottom - rect.top,
			nullptr, NULL, hInstance, NULL
		);
		CreateWindow("EDIT", pOut, WS_CHILD | WS_VISIBLE | WS_BORDER,
			DialogTextX, DialogControlY, DialogTextWidth, DialogControlHeight,
			hWnd, (HMENU)CONTROL_ID_TEXT, hInstance, NULL);
		CreateWindow("BUTTON", "OK", WS_CHILD | WS_VISIBLE | WS_BORDER,
			DialogButtonX, DialogControlY, DialogButtonWidth, DialogControlHeight,
			hWnd, (HMENU)CONTROL_ID_BUTTON, hInstance, NULL);

		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
		g_dialogTextReason = DIALOG_TEXT_OPEN;
		break;
	}

	case DIALOG_TEXT_OK:
		strcpy_s(pOut, strnlen_s(pOut, CONTROL_TEXT_LEN), g_controlText);
		g_dialogTextReason = DIALOG_TEXT_CLOSE;
		return DIALOG_TEXT_OK;

	case DIALOG_TEXT_CANCEL:
		strcpy_s(pOut, 2, "");
		g_dialogTextReason = DIALOG_TEXT_CLOSE;
		return DIALOG_TEXT_CANCEL;
	}

	return g_dialogTextReason;
}


// �e�L�X�g�_�C�A���O�p�v���V�[�W��
LRESULT CALLBACK DialogTextWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case CONTROL_ID_BUTTON:
			DestroyWindow(hWnd);
			g_dialogTextReason = DIALOG_TEXT_OK;
			break;
		case CONTROL_ID_TEXT:
			switch (HIWORD(wParam))
			{
			case EN_SETFOCUS:
				PostMessage(GetFocus(), EM_SETSEL, 0, -1);
				break;
			case EN_UPDATE:
				GetWindowText(GetFocus(), g_controlText, CONTROL_TEXT_LEN);
				break;
			}
			break;
		}
		break;

	case WM_CLOSE:
		g_dialogTextReason = DIALOG_TEXT_CANCEL;
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
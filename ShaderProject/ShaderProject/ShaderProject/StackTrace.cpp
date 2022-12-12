/*
* @remarks �X�^�b�N�g���[�X
*	https://an-embedded-engineer.hateblo.jp/entry/2020/08/24/212511
*	https://gameworkslab.jp/2019/01/25/backtrace/
*	https://black-techmemo.net/win32api/1551/
* @remarks �V���{���t�@�C��
*	https://docs.microsoft.com/ja-jp/windows/win32/debug/symbol-files
*	https://docs.microsoft.com/ja-jp/windows-hardware/drivers/debugger/symbols-and-symbol-files
* �f�o�b�O�v���Z�X�ɗ��p�����f�[�^�B.pdb�t�@�C���ɃV���{�����ێ�����Ă���B
*/ 

#include "StackTrace.h"
#include <Windows.h>
#include <ImageHlp.h>

#pragma comment(lib, "imagehlp.lib")

StackTrace GetStackTrace()
{
	const DWORD MaxTrace = 64;
	const int MaxSymbolName = 256;

	void* trace[MaxTrace] = {};
	StackTrace stackTrace;

	// �X�^�b�N�g���[�X�̎擾
	WORD traceSize = CaptureStackBackTrace(
		0,			// �֐��̌Ăяo���K�w�i�O�Ō��݂̊֐� 
		MaxTrace,	// �g���[�X�̍ő�[�x
		trace,		// �g���[�X����(�A�h���X)�i�[��
		NULL);

	// �V���{���n���h���̏�����
	HANDLE process = GetCurrentProcess();
	SymInitialize(process, NULL, true);

	// �V���{�����̊i�[���p��
	SYMBOL_INFO* pSymbol = reinterpret_cast<SYMBOL_INFO*>(new char[sizeof(SYMBOL_INFO) + MaxSymbolName]);
	pSymbol->MaxNameLen = MaxSymbolName;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	// �X�^�b�N�g���[�X�̏����V���{���Ɋ��蓖��
	DWORD64 dispAddr;
	IMAGEHLP_LINE64 line = { sizeof(IMAGEHLP_LINE64) };
	DWORD dispLine;
	for(int i = 1; i < traceSize; ++i)
	{
		// �g���[�X���ʂ̃A�h���X���V���{���Ɋ��蓖��
		SymFromAddr(process, DWORD64(trace[i]), &dispAddr, pSymbol);
		SymGetLineFromAddr64(process, pSymbol->Address + dispAddr, &dispLine, &line);
		// invoke_main�܂Ńg���[�X������I��
		if (!strcmp(pSymbol->Name, "invoke_main"))
		{
			break;
		}
		// �V���{������g���[�X�����擾
		std::string file = line.FileName;
		TraceLog log;
		log.line = line.LineNumber;
		log.filePath = file.substr(0, file.find_last_of("\\"));
		log.fileName = file.substr(file.find_last_of("\\") + 1);
		log.funcName = pSymbol->Name;
		stackTrace.push_back(log);
	}

	delete[] pSymbol;

	return stackTrace;
}
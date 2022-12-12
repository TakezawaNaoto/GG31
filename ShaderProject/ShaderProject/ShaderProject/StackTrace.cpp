/*
* @remarks スタックトレース
*	https://an-embedded-engineer.hateblo.jp/entry/2020/08/24/212511
*	https://gameworkslab.jp/2019/01/25/backtrace/
*	https://black-techmemo.net/win32api/1551/
* @remarks シンボルファイル
*	https://docs.microsoft.com/ja-jp/windows/win32/debug/symbol-files
*	https://docs.microsoft.com/ja-jp/windows-hardware/drivers/debugger/symbols-and-symbol-files
* デバッグプロセスに利用されるデータ。.pdbファイルにシンボルが保持されている。
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

	// スタックトレースの取得
	WORD traceSize = CaptureStackBackTrace(
		0,			// 関数の呼び出し階層（０で現在の関数 
		MaxTrace,	// トレースの最大深度
		trace,		// トレース結果(アドレス)格納先
		NULL);

	// シンボルハンドラの初期化
	HANDLE process = GetCurrentProcess();
	SymInitialize(process, NULL, true);

	// シンボル情報の格納先を用意
	SYMBOL_INFO* pSymbol = reinterpret_cast<SYMBOL_INFO*>(new char[sizeof(SYMBOL_INFO) + MaxSymbolName]);
	pSymbol->MaxNameLen = MaxSymbolName;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	// スタックトレースの情報をシンボルに割り当て
	DWORD64 dispAddr;
	IMAGEHLP_LINE64 line = { sizeof(IMAGEHLP_LINE64) };
	DWORD dispLine;
	for(int i = 1; i < traceSize; ++i)
	{
		// トレース結果のアドレスをシンボルに割り当て
		SymFromAddr(process, DWORD64(trace[i]), &dispAddr, pSymbol);
		SymGetLineFromAddr64(process, pSymbol->Address + dispAddr, &dispLine, &line);
		// invoke_mainまでトレースしたら終了
		if (!strcmp(pSymbol->Name, "invoke_main"))
		{
			break;
		}
		// シンボルからトレース情報を取得
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
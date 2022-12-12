#ifndef __WIN_DIALOG_H__
#define __WIN_DIALOG_H__

enum DialogTextReason
{
	DIALOG_TEXT_INITIALIZE,
	DIALOG_TEXT_FAILED,
	DIALOG_TEXT_CLOSE,
	DIALOG_TEXT_CREATE,
	DIALOG_TEXT_OPEN,
	DIALOG_TEXT_OK,
	DIALOG_TEXT_CANCEL,
};

#define STACK_DEFAULT (1)

void DialogMessage(unsigned char stacktrace, const char* text, ...);
void DialogError(unsigned char stacktrace, const char* text, ...);
DialogTextReason DialogText(char* pOut, int outSize);


#endif // __WIN_DIALOG_H__
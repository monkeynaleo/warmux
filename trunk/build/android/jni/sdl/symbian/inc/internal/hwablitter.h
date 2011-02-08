#ifndef HWABLITTER
#define HWABLITTER

#include <sdlepocapi.h>
#include <gdi.h>

class HWAImp;
class RWindow;

NONSHARABLE_CLASS(CBlitter) : public CBase, public MBlitter
 {
public:
     virtual void Release() {}
 };


NONSHARABLE_CLASS(CHWABlitter) : public CBlitter
	{
public:
	enum{ENoScale = 0x0, EPreventScale = 0x1, EPreserveRatio = 0x2};
	static CHWABlitter* NewL(RWindow& aWindow, TInt aFlags);
	~CHWABlitter();
	TBool BitBlt(CBitmapContext& aGc,CFbsBitmap& aBmp, const TRect& aTargetRect, const TSize& aSize);
private:
	CHWABlitter(TInt aFlags);
	void Release();
private:
	TInt iFlags;
	HWAImp* iImp;
	};
	
	
#endif

/*
 *	PostScript driver text functions
 *
 *	Copyright 1998  Huw D M Davies
 *
 */
#include <string.h>
#include "psdrv.h"
#include "debugtools.h"
#include "winspool.h"

DEFAULT_DEBUG_CHANNEL(psdrv)

static BOOL PSDRV_Text(DC *dc, INT x, INT y, LPCSTR str, UINT count);

/***********************************************************************
 *           PSDRV_ExtTextOut
 */
BOOL PSDRV_ExtTextOut( DC *dc, INT x, INT y, UINT flags,
		       const RECT *lprect, LPCSTR str, UINT count,
		       const INT *lpDx )
{
    PSDRV_PDEVICE *physDev = (PSDRV_PDEVICE *)dc->physDev;
    BOOL bResult = TRUE;
    RECT rect;

    TRACE("(x=%d, y=%d, flags=0x%08x, str='%.*s', count=%d)\n", x, y,
	  flags, (int)count, str, count);

    /* write font if not already written */
    PSDRV_SetFont(dc);

    /* set clipping and/or draw background */
    if ((flags & (ETO_OPAQUE | ETO_CLIPPED)) && (lprect != NULL))
    {
	rect.left = XLPTODP(dc, lprect->left);
	rect.right = XLPTODP(dc, lprect->right);
	rect.top = YLPTODP(dc, lprect->top);
	rect.bottom = YLPTODP(dc, lprect->bottom);

	PSDRV_WriteGSave(dc);
	PSDRV_WriteRectangle(dc, rect.left, rect.top, rect.right - rect.left, 
			     rect.bottom - rect.top);

	if (flags & ETO_OPAQUE)
	{
	    PSDRV_WriteGSave(dc);
	    PSDRV_WriteSetColor(dc, &physDev->bkColor);
	    PSDRV_WriteFill(dc);
	    PSDRV_WriteGRestore(dc);
	}

	if (flags & ETO_CLIPPED)
	{
	    PSDRV_WriteClip(dc);
	}

	bResult = PSDRV_Text(dc, x, y, str, count); 
	PSDRV_WriteGRestore(dc);
    }
    else
    {
	bResult = PSDRV_Text(dc, x, y, str, count); 
    }

    return bResult;
}

/***********************************************************************
 *           PSDRV_Text
 */
static BOOL PSDRV_Text(DC *dc, INT x, INT y, LPCSTR str, UINT count)
{
    PSDRV_PDEVICE *physDev = (PSDRV_PDEVICE *)dc->physDev;
    char *strbuf;
    SIZE sz;
    POINT pt;

    strbuf = (char *)HeapAlloc( PSDRV_Heap, 0, count + 1);
    if(!strbuf) {
        WARN("HeapAlloc failed\n");
        return FALSE;
    }

    if(dc->w.textAlign & TA_UPDATECP) {
	x = dc->w.CursPosX;
	y = dc->w.CursPosY;
    }

    pt.x = x = XLPTODP(dc, x);
    pt.y = y = YLPTODP(dc, y);

    GetTextExtentPoint32A(dc->hSelf, str, count, &sz);
    sz.cx = XLSTODS(dc, sz.cx);
    sz.cy = YLSTODS(dc, sz.cy);

    switch(dc->w.textAlign & (TA_LEFT | TA_CENTER | TA_RIGHT) ) {
    case TA_LEFT:
        if(dc->w.textAlign & TA_UPDATECP)
	    dc->w.CursPosX = XDPTOLP(dc, x + sz.cx);
	break;

    case TA_CENTER:
	x -= sz.cx/2;
	break;

    case TA_RIGHT:
	x -= sz.cx;
	if(dc->w.textAlign & TA_UPDATECP)
	    dc->w.CursPosX = XDPTOLP(dc, x);
	break;
    }

    switch(dc->w.textAlign & (TA_TOP | TA_BASELINE | TA_BOTTOM) ) {
    case TA_TOP:
        y += physDev->font.tm.tmAscent;
	break;

    case TA_BASELINE:
	break;

    case TA_BOTTOM:
        y -= physDev->font.tm.tmDescent;
	break;
    }

    memcpy(strbuf, str, count);
    *(strbuf + count) = '\0';
    
    PSDRV_SetFont(dc);

    PSDRV_WriteGSave(dc);
    PSDRV_WriteNewPath(dc);
    PSDRV_WriteRectangle(dc, pt.x, pt.y, sz.cx, sz.cy);
    PSDRV_WriteSetColor(dc, &physDev->bkColor);
    PSDRV_WriteFill(dc);
    PSDRV_WriteGRestore(dc);
    PSDRV_WriteMoveTo(dc, x, y);
    PSDRV_WriteShow(dc, strbuf, strlen(strbuf));

    /*
     * Underline and strikeout attributes.
     */
    if ((physDev->font.tm.tmUnderlined) || (physDev->font.tm.tmStruckOut)) {

        /* Get the thickness and the position for the underline attribute */
        /* We'll use the same thickness for the strikeout attribute       */

        float thick = physDev->font.afm->UnderlineThickness * physDev->font.scale;
        float pos   = -physDev->font.afm->UnderlinePosition * physDev->font.scale;
        SIZE size;
        INT escapement =  physDev->font.escapement;

        TRACE("Position = %f Thickness %f Escapement %d\n",
              pos, thick, escapement);

        /* Get the width of the text */

        PSDRV_GetTextExtentPoint(dc, strbuf, strlen(strbuf), &size);
        size.cx = XLSTODS(dc, size.cx);

        /* Do the underline */

        if (physDev->font.tm.tmUnderlined) {
            if (escapement != 0)  /* rotated text */
            {
                PSDRV_WriteGSave(dc);  /* save the graphics state */
                PSDRV_WriteMoveTo(dc, x, y); /* move to the start */

                /* temporarily rotate the coord system */
                PSDRV_WriteRotate(dc, -escapement/10); 
                
                /* draw the underline relative to the starting point */
                PSDRV_WriteRRectangle(dc, 0, (INT)pos, size.cx, (INT)thick);
            }
            else
                PSDRV_WriteRectangle(dc, x, y + (INT)pos, size.cx, (INT)thick);

            PSDRV_WriteFill(dc);

            if (escapement != 0)  /* rotated text */
                PSDRV_WriteGRestore(dc);  /* restore the graphics state */
        }

        /* Do the strikeout */

        if (physDev->font.tm.tmStruckOut) {
            pos = -physDev->font.tm.tmAscent / 2;

            if (escapement != 0)  /* rotated text */
            {
                PSDRV_WriteGSave(dc);  /* save the graphics state */
                PSDRV_WriteMoveTo(dc, x, y); /* move to the start */

                /* temporarily rotate the coord system */
                PSDRV_WriteRotate(dc, -escapement/10);

                /* draw the underline relative to the starting point */
                PSDRV_WriteRRectangle(dc, 0, (INT)pos, size.cx, (INT)thick);
            }
            else
                PSDRV_WriteRectangle(dc, x, y + (INT)pos, size.cx, (INT)thick);

            PSDRV_WriteFill(dc);

            if (escapement != 0)  /* rotated text */
                PSDRV_WriteGRestore(dc);  /* restore the graphics state */
        }
    }

    HeapFree(PSDRV_Heap, 0, strbuf);
    return TRUE;
}

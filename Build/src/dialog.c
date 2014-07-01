// ---------------------- 2009 ----------------------
// Version 1.2
//

#define DIRECTINPUT_VERSION 0x0700
#define DIRECTDRAW_VERSION  0x0600

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <dinput.h>
#include <stdlib.h>

#include "winlayer.h"   // only thing here is for hInstanceLocal = win_gethinstance() (apps handle);
#include "editor.h"     // for configs
#include "dialog.h"

void PopulateConfigDialog(HWND hwnd);
void DoConfigs(HWND hwndcfg);
void SWPBUILDConfig(void);

static HWND startupdlog = NULL;
static int  startupdlogcommand = 0;
static void (*startupdlogonclose)(void) = NULL;

short KeySelect = 0;

extern short brightness; // for config

// startup_dialogproc() -- dialog procedure for the initialisation dialog

static INT_PTR CALLBACK startup_dialogproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP hbmp = NULL;
	int iKey;
	RECT rdlg, rbmp;
	HDC hdc;

    HINSTANCE hInstanceLocal = win_gethinstance();

	switch (uMsg)
	{
		case WM_INITDIALOG:
			// set the bitmap
			hbmp = LoadBitmap(hInstanceLocal, MAKEINTRESOURCE(WIN_DIALOGWINBMP));
			SendDlgItemMessage(hwndDlg, WIN_DIALOGWIN_ITEMBITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmp);
			//SetClassLong(hwndDlg, GCL_HICON, (LONG)LoadIcon(hInstanceLocal, MAKEINTRESOURCE(WIN_DIALOGWINBMP)));

			// fetch the adjusted size
			GetClientRect(GetDlgItem(hwndDlg,WIN_DIALOGWIN_ITEMBITMAP), &rbmp);
			rbmp.right++; rbmp.bottom++;

			// move the bitmap to the top of the client area
			MoveWindow(GetDlgItem(hwndDlg,WIN_DIALOGWIN_ITEMBITMAP),
					0,0, rbmp.right,rbmp.bottom,
					FALSE);

			rdlg.left = 0;
			rdlg.top = 0;
			rdlg.right = rbmp.right+DIALOGWIDE;
			rdlg.bottom = rbmp.bottom;

			AdjustWindowRect(&rdlg,	GetWindowLong(hwndDlg, GWL_STYLE), FALSE); // sets bmp height

			rdlg.right -= rdlg.left;
			rdlg.bottom -= rdlg.top;

			hdc = GetDC(NULL);
			rdlg.left = (GetDeviceCaps(hdc, HORZRES) - rdlg.right) / 2;
			rdlg.top = (GetDeviceCaps(hdc, VERTRES) - rdlg.bottom) / 2;
			ReleaseDC(NULL, hdc);

			SetWindowPos(hwndDlg,NULL, rdlg.left,rdlg.top, rdlg.right,rdlg.bottom, SWP_NOREPOSITION); // total win height
            CheckDlgButton(hwndDlg, ID_QUITSAVE, TRUE);   // check first radiobutton as default

            SetFocus(hwndDlg);
            MessageBeep(48);
			return TRUE;

		case WM_CLOSE:
			quitevent = 1;
			return TRUE;

		case WM_DESTROY:
			if (hbmp)
			{
				DeleteObject(hbmp);
				hbmp = NULL;
			}
			if (startupdlogonclose)
			    startupdlogonclose();
			startupdlog = NULL;
			return TRUE;

		case WM_COMMAND:
		    iKey = wParam;               // iKey: 2=esc / 1 to 9 = 301 to 309
		    if (iKey == 2)               // escape
		       {
               KeySelect = 0;
               EndDialog(hwndDlg, 1);
		       return TRUE;
		       }
		    switch (wParam)
		    {
		        case WM_KEYDOWN:         // not working for some odd reason ?
	            case WM_KEYUP:
		             return TRUE;

		        case ID_QUITSAVE:
		             KeySelect = 1;
		             return TRUE;

		        case ID_QUITNOSAVE:
		             KeySelect = 2;
		             return TRUE;

		        case ID_PLAYMAP:
		             KeySelect = 3;
		             return TRUE;

		        case ID_SAVEMAP:
		             KeySelect = 4;
		             return TRUE;

		        case ID_VIEW64:
		             KeySelect = 5;
		             return TRUE;

		        case ID_VIEW128:
		             KeySelect = 6;
		             return TRUE;

		        case ID_VIEW256:
		             KeySelect = 7;

		             return TRUE;
				case ID_HELP:
				    KeySelect = 20;
				    EndDialog(hwndDlg, 1);
					return TRUE;

				case ID_CONFIG:
				    KeySelect = 21;
				    EndDialog(hwndDlg, 1);
					return TRUE;

				case ID_SUBMIT:
				    EndDialog(hwndDlg, 1);
					return TRUE;

				case ID_CANCEL:
					KeySelect = 0;
					EndDialog(hwndDlg, 1);
					return TRUE;
            }
            break;
		default:
		  break;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------

static INT_PTR CALLBACK config_dialogproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP hbmp = NULL;
	int iKey;
	RECT rdlg, rbmp;
	HDC hdc;

    HINSTANCE hInstanceLocal = win_gethinstance();

	switch (uMsg)
	{
		case WM_INITDIALOG:
			// set the bitmap
			hbmp = LoadBitmap(hInstanceLocal, MAKEINTRESOURCE(WIN_CONFIGWINBMP));
			SendDlgItemMessage(hwndDlg, WIN_CONFIGWIN_ITEMBITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmp);
			PopulateConfigDialog(hwndDlg);
			//SetClassLong(hwndDlg, GCL_HICON, (LONG)LoadIcon(hInstanceLocal, MAKEINTRESOURCE(WIN_CONFIGWINBMP)));

			// fetch the adjusted size
			GetClientRect(GetDlgItem(hwndDlg,WIN_CONFIGWIN_ITEMBITMAP), &rbmp);
			rbmp.right++; rbmp.bottom++;

			// move the bitmap to the top of the client area
			MoveWindow(GetDlgItem(hwndDlg,WIN_CONFIGWIN_ITEMBITMAP),
					0,0, rbmp.right,rbmp.bottom,
					FALSE);

			rdlg.left = 0;
			rdlg.top = 0;
			rdlg.right = rbmp.right+DIALOGWIDE;
			rdlg.bottom = rbmp.bottom;

			AdjustWindowRect(&rdlg,	GetWindowLong(hwndDlg, GWL_STYLE), FALSE); // sets bmp height

			rdlg.right -= rdlg.left;
			rdlg.bottom -= rdlg.top;

			hdc = GetDC(NULL);
			rdlg.left = (GetDeviceCaps(hdc, HORZRES) - rdlg.right) / 2;
			rdlg.top = (GetDeviceCaps(hdc, VERTRES) - rdlg.bottom) / 2;
			ReleaseDC(NULL, hdc);

			SetWindowPos(hwndDlg,NULL, rdlg.left,rdlg.top, rdlg.right,rdlg.bottom, SWP_NOREPOSITION); // total win height
            //CheckDlgButton(hwndDlg, ID_KEY01, TRUE);   // check first radiobutton as default

            SetFocus(hwndDlg);
			return TRUE;

		case WM_CLOSE:
			quitevent = 1;
			return TRUE;

		case WM_DESTROY:
			if (hbmp)
			{
				DeleteObject(hbmp);
				hbmp = NULL;
			}
			if (startupdlogonclose)
			    startupdlogonclose();
			startupdlog = NULL;
			return TRUE;

		case WM_COMMAND:
		    iKey = wParam;               // iKey: 2=esc / 1 to 9 = 301 to 309
		    if (iKey == 2)               // escape
		       {
               KeySelect = 0;
               EndDialog(hwndDlg, 1);
		       return TRUE;
		       }
		    switch (wParam)
		    {
		        case WM_KEYDOWN:         // not working for some odd reason ?
	            case WM_KEYUP:
	                 MessageBeep(24);
		             return TRUE;

				case ID_ACCEPT:
				    KeySelect = 99;
				    DoConfigs(hwndDlg);
				    EndDialog(hwndDlg, 1);
					return TRUE;

				case ID_CANSEL:
					KeySelect = 0;
					EndDialog(hwndDlg, 1);
					return TRUE;
            }
            break;
		default:
		  break;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------

void PopulateConfigDialog(HWND hwnd)
{
    HWND hwndCmb;
    int i;

    hwndCmb = GetDlgItem(hwnd,ID_FULLSCREEN);
    if (fullscreen == 1)
        CheckDlgButton(hwndCmb, ID_FULLSCREEN, TRUE);

    hwndCmb = GetDlgItem(hwnd,ID_VIDEORES);
	ComboBox_ResetContent(hwndCmb);
	ComboBox_AddString(hwndCmb, "640 x 480");
	ComboBox_AddString(hwndCmb, "800 x 600");
	ComboBox_AddString(hwndCmb, "1024 x 768");
	i = 0;
	if (xdimgame == 800)
	    i = 1;
	else
	if (xdimgame > 1000)
	    i = 2;
    ComboBox_SetCurSel(hwndCmb, i);

    hwndCmb = GetDlgItem(hwnd,ID_VIDEOBPP);
	ComboBox_ResetContent(hwndCmb);
	ComboBox_AddString(hwndCmb, "8");
	ComboBox_AddString(hwndCmb, "16");
	ComboBox_AddString(hwndCmb, "32");
	i = 0;
	if (bppgame == 16)
	    i = 1;
	else
	if (bppgame > 16)
	    i = 2;
    ComboBox_SetCurSel(hwndCmb, i);

    hwndCmb = GetDlgItem(hwnd,ID_VREFRESH);
	ComboBox_ResetContent(hwndCmb);
	ComboBox_AddString(hwndCmb, "60");
	ComboBox_AddString(hwndCmb, "70");
	ComboBox_AddString(hwndCmb, "75");
	i = 0;
	if (maxrefreshfreq == 70)
	    i = 1;
	else
	if (maxrefreshfreq > 70)
	    i = 2;
    ComboBox_SetCurSel(hwndCmb, i);

    hwndCmb = GetDlgItem(hwnd,ID_VBRIGHT);
    ComboBox_ResetContent(hwndCmb);
	ComboBox_AddString(hwndCmb, "0");
	ComboBox_AddString(hwndCmb, "3");
	ComboBox_AddString(hwndCmb, "6");
	ComboBox_AddString(hwndCmb, "9");
	ComboBox_AddString(hwndCmb, "12");
	ComboBox_AddString(hwndCmb, "15");
	i = brightness / 3;
	ComboBox_SetCurSel(hwndCmb, i);
}

int win_getstartupcommand(void)
{
	int t = startupdlogcommand;
	startupdlogcommand = 0;
	return t;
}

//-----------------------------------------------------------------------------

// The main access point - called from app to open dialog

int dialogwin_open(short x)
{
    HWND hwndStart;
    long saferect[4];

	if (startupdlog)
	    return 1;

	if (x == 0)
	    startupdlog = DialogBox((HINSTANCE)win_gethinstance(), MAKEINTRESOURCE(WIN_DIALOGWIN), NULL, startup_dialogproc);
	else
	    startupdlog = DialogBox((HINSTANCE)win_gethinstance(), MAKEINTRESOURCE(WIN_CONFIGWIN), NULL, config_dialogproc);

	if (startupdlog)
		return 0;
	return -1;
}

int dialogwin_close(void)
{
	if (!startupdlog) return 1;
	DestroyWindow(startupdlog);
	startupdlog = NULL;
	return 0;
}

int GetMenuOption(void)
{
    HWND myHandle = win_gethwnd();

    dialogwin_open(0);
    dialogwin_close();
    EndDialog(startupdlog, 1);
    SetFocus(myHandle);
    if (KeySelect == 21)
       {
       KeySelect = 0;
       SWPBUILDConfig();
       }
    return KeySelect;
}

void SWPBUILDConfig(void)
{
    HWND myHandle = win_gethwnd();

    dialogwin_open(1);
    dialogwin_close();
    EndDialog(startupdlog, 1);
    SetFocus(myHandle);
}

void DoConfigs(HWND hwndcfg)
{
    char sTemp[4];
    short x;

    if (IsDlgButtonChecked(hwndcfg,ID_FULLSCREEN))
        fullscreen = 1;
    else
        fullscreen = 0;

    //ComboBox_GetCurSel(GetDlgItemText(hwndcfg,ID_VIDEORES,sTemp,12));
    x = ComboBox_GetCurSel(GetDlgItem(hwndcfg,ID_VIDEORES));
    if (x == 0)
       {
       xdim2d = 640;
       ydim2d = 480;
       }
    else
    if (x == 1)
       {
       xdim2d = 800;
       ydim2d = 600;
       }
    else
       {
       xdim2d = 1024;
       ydim2d = 768;
       }
    xdimgame = xdim2d;
    ydimgame = ydim2d;

    ComboBox_GetCurSel(GetDlgItemText(hwndcfg,ID_VIDEOBPP,sTemp,3));
    bppgame = atoi(sTemp);

    ComboBox_GetCurSel(GetDlgItemText(hwndcfg,ID_VREFRESH,sTemp,3));
    maxrefreshfreq = atoi(sTemp);

    ComboBox_GetCurSel(GetDlgItemText(hwndcfg,ID_VBRIGHT,sTemp,3));
    brightness = atoi(sTemp);

    writesetup("SWPBUILD.cfg");
    MessageBeep(64);
}


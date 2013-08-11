#import <Cocoa/Cocoa.h>

#include "baselayer.h"
#include "build.h"

static struct {
	int fullscreen;
	int xdim2d, ydim2d;
	int xdim3d, ydim3d, bpp3d;
	int forcesetup;
} settings;

@interface StartupWinController : NSWindowController
{
	NSMutableArray *modeslist2d;
	NSMutableArray *modeslist3d;

	IBOutlet NSButton *alwaysShowButton;
	IBOutlet NSButton *fullscreenButton;
	IBOutlet NSTextView *messagesView;
	IBOutlet NSTabView *tabView;
	IBOutlet NSPopUpButton *videoMode2DPUButton;
	IBOutlet NSPopUpButton *videoMode3DPUButton;
	
	IBOutlet NSButton *cancelButton;
	IBOutlet NSButton *startButton;
}

- (void)dealloc;
- (void)populateVideoModes:(BOOL)firstTime;

- (IBAction)alwaysShowClicked:(id)sender;
- (IBAction)fullscreenClicked:(id)sender;

- (IBAction)cancel:(id)sender;
- (IBAction)start:(id)sender;

- (void)setupRunMode;
- (void)setupMessagesMode;
- (void)putsMessage:(NSString *)str;
- (void)setTitle:(NSString *)str;
@end

@implementation StartupWinController

- (void)dealloc
{
	[modeslist2d release];
	[modeslist3d release];
	[super dealloc];
}

- (void)populateVideoModes:(BOOL)firstTime
{
	int i, mode2d, mode3d, fullscreen = ([fullscreenButton state] == NSOnState);
	int idx2d = -1, idx3d = -1;
	int xdim, ydim, bpp;
	
	if (firstTime) {
		xdim = settings.xdim2d;
		ydim = settings.ydim2d;
	} else {
		mode2d = [[modeslist2d objectAtIndex:[videoMode2DPUButton indexOfSelectedItem]] intValue];
		if (mode2d >= 0) {
			xdim = validmode[mode2d].xdim;
			ydim = validmode[mode2d].ydim;
		}
	}
	mode2d = checkvideomode(&xdim, &ydim, 8, fullscreen, 1);
	if (mode2d < 0) mode2d = 0;

	if (firstTime) {
		xdim = settings.xdim3d;
		ydim = settings.ydim3d;
		bpp  = settings.bpp3d;
	} else {
		mode3d = [[modeslist3d objectAtIndex:[videoMode3DPUButton indexOfSelectedItem]] intValue];
		if (mode3d >= 0) {
			xdim = validmode[mode3d].xdim;
			ydim = validmode[mode3d].ydim;
			bpp = validmode[mode3d].bpp;
		}
		
	}
	mode3d = checkvideomode(&xdim, &ydim, bpp, fullscreen, 1);
	if (mode3d < 0) {
		int i, cd[] = { 32, 24, 16, 15, 8, 0 };
		for (i=0; cd[i]; ) { if (cd[i] >= bpp) i++; else break; }
		for ( ; cd[i]; i++) {
			mode3d = checkvideomode(&xdim, &ydim, cd[i], fullscreen, 1);
			if (mode3d < 0) continue;
			break;
		}
	}
	
	[modeslist2d release];
	[modeslist3d release];
	[videoMode2DPUButton removeAllItems];
	[videoMode3DPUButton removeAllItems];

	modeslist2d = [[NSMutableArray alloc] init];
	modeslist3d = [[NSMutableArray alloc] init];

	for (i = 0; i < validmodecnt; i++) {
		if (fullscreen == validmode[i].fs) {
			if (validmode[i].bpp == 8 && validmode[i].xdim >= 640 && validmode[i].ydim >= 480) {
				if (i == mode2d) idx2d = [modeslist2d count];
				[modeslist2d addObject:[NSNumber numberWithInt:i]];
				[videoMode2DPUButton addItemWithTitle:[NSString stringWithFormat:@"%d %C %d",
					validmode[i].xdim, 0xd7, validmode[i].ydim]];
			}

			if (i == mode3d) idx3d = [modeslist3d count];
			[modeslist3d addObject:[NSNumber numberWithInt:i]];
			[videoMode3DPUButton addItemWithTitle:[NSString stringWithFormat:@"%d %C %d %d-bpp",
				validmode[i].xdim, 0xd7, validmode[i].ydim, validmode[i].bpp]];
		}
	}

	if (idx2d >= 0) [videoMode2DPUButton selectItemAtIndex:idx2d];
	if (idx3d >= 0) [videoMode3DPUButton selectItemAtIndex:idx3d];
}

- (IBAction)alwaysShowClicked:(id)sender
{
}

- (IBAction)fullscreenClicked:(id)sender
{
	[self populateVideoModes:NO];
}

- (IBAction)cancel:(id)sender
{
	[NSApp abortModal];
}

- (IBAction)start:(id)sender
{
	int mode = [[modeslist3d objectAtIndex:[videoMode3DPUButton indexOfSelectedItem]] intValue];
	if (mode >= 0) {
		settings.xdim3d = validmode[mode].xdim;
		settings.ydim3d = validmode[mode].ydim;
		settings.bpp3d = validmode[mode].bpp;
		settings.fullscreen = validmode[mode].fs;
	}
	
	mode = [[modeslist2d objectAtIndex:[videoMode2DPUButton indexOfSelectedItem]] intValue];
	if (mode >= 0) {
		settings.xdim2d = validmode[mode].xdim;
		settings.ydim2d = validmode[mode].ydim;
	}
	
	settings.forcesetup = [alwaysShowButton state] == NSOnState;

	[NSApp stopModal];
}

- (void)setupRunMode
{
	getvalidmodes();

	[fullscreenButton setState: (settings.fullscreen ? NSOnState : NSOffState)];
	[alwaysShowButton setState: (settings.forcesetup ? NSOnState : NSOffState)];
	[self populateVideoModes:YES];

	// enable all the controls on the Configuration page
	NSEnumerator *enumerator = [[[[tabView tabViewItemAtIndex:0] view] subviews] objectEnumerator];
	NSControl *control;
	while (control = [enumerator nextObject]) [control setEnabled:true];
	
	[cancelButton setEnabled:true];
	[startButton setEnabled:true];

	[tabView selectTabViewItemAtIndex:0];
	[NSCursor unhide];	// Why should I need to do this?
}

- (void)setupMessagesMode
{
	[tabView selectTabViewItemAtIndex:1];

	// disable all the controls on the Configuration page except "always show", so the
	// user can enable it if they want to while waiting for something else to happen
	NSEnumerator *enumerator = [[[[tabView tabViewItemAtIndex:0] view] subviews] objectEnumerator];
	NSControl *control;
	while (control = [enumerator nextObject]) {
		if (control == alwaysShowButton) continue;
		[control setEnabled:false];
	}

	[cancelButton setEnabled:false];
	[startButton setEnabled:false];
}

- (void)putsMessage:(NSString *)str
{
	NSRange end;
	NSTextStorage *text = [messagesView textStorage];
	BOOL shouldAutoScroll;

	shouldAutoScroll = ((int)NSMaxY([messagesView bounds]) == (int)NSMaxY([messagesView visibleRect]));

	end.location = [text length];
	end.length = 0;

	[text beginEditing];
	[messagesView replaceCharactersInRange:end withString:str];
	[text endEditing];
	
	if (shouldAutoScroll) {
		end.location = [text length];
		end.length = 0;
		[messagesView scrollRangeToVisible:end];
	}
}

- (void)setTitle:(NSString *)str
{
	[[self window] setTitle:str];
}

@end

static StartupWinController *startwin = nil;

int startwin_open(void)
{
	if (startwin != nil) return 1;
	
	startwin = [[StartupWinController alloc] initWithWindowNibName:@"startwin.editor"];
	if (startwin == nil) return -1;

	[startwin setupMessagesMode];
	[startwin showWindow:nil];

	return 0;
}

int startwin_close(void)
{
	if (startwin == nil) return 1;

	[startwin close];
	[startwin release];
	startwin = nil;

	return 0;
}

int startwin_puts(const char *s)
{
	NSString *ns;

	if (!s) return -1;
	if (startwin == nil) return 1;

	ns = [[NSString alloc] initWithCString:s];
	[startwin putsMessage:ns];
	[ns release];

	return 0;
}

int startwin_settitle(const char *s)
{
	NSString *ns;
	
	if (!s) return -1;
	if (startwin == nil) return 1;
	
	ns = [[NSString alloc] initWithCString:s];
	[startwin setTitle:ns];
	[ns release];

	return 0;
}

int startwin_idle(void *v)
{
	if (startwin) [[startwin window] displayIfNeeded];
	return 0;
}

extern int xdim2d, ydim2d, xdimgame, ydimgame, bppgame, forcesetup;

int startwin_run(void)
{
	int retval;
	
	if (startwin == nil) return 0;

	settings.fullscreen = fullscreen;
	settings.xdim2d = xdim2d;
	settings.ydim2d = ydim2d;
	settings.xdim3d = xdimgame;
	settings.ydim3d = ydimgame;
	settings.bpp3d = bppgame;
	settings.forcesetup = forcesetup;
	
	[startwin setupRunMode];
	
	switch ([NSApp runModalForWindow:[startwin window]]) {
		case NSRunStoppedResponse: retval = 1; break;
		case NSRunAbortedResponse: retval = 0; break;
		default: retval = -1;
	}
	
	[startwin setupMessagesMode];

	if (retval) {
		fullscreen = settings.fullscreen;
		xdim2d = settings.xdim2d;
		ydim2d = settings.ydim2d;
		xdimgame = settings.xdim3d;
		ydimgame = settings.ydim3d;
		bppgame = settings.bpp3d;
		forcesetup = settings.forcesetup;
	}
	
	return retval;
}

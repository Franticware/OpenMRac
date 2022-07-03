#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>

// inspired by SDL 2.0

FILE* fopenDir(const char* filename, const char* mode, const char* org, const char* app)
{
	char filepath[1024] = {0};
	int i;
	if (org && app)
	{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		NSArray *array = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);	
		if ([array count] > 0) 
		{  /* we only want the first item in the list. */
			NSString *str = [array objectAtIndex:0];
			const char *base = [str fileSystemRepresentation];
			snprintf(filepath, 1023, "%s/%s/%s/%s", base, org, app, filename);
		}
		[pool release];
	}
	else
	{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		NSString *str = NSHomeDirectory();
		const char *base = [str fileSystemRepresentation];
		snprintf(filepath, 1023, "%s/%s", base, filename);
		[pool release];
	}
	//fprintf(stderr, "%s\n", filepath);
	for (i = 1; i < (int)(strlen(filepath)); ++i)
    {
        if (filepath[i] == '/')
        {
            filepath[i] = '\0';
            mkdir(filepath, 0700);
            filepath[i] = '/';
        }
    }
    return fopen(filepath, mode);
}
